"""
Authselect SSSD Profile Test Cases

:requirement: authselect
"""
from __future__ import annotations

import pytest
from sssd_test_framework.roles.client import Client
from sssd_test_framework.roles.generic import GenericProvider
from sssd_test_framework.topology import KnownTopology, KnownTopologyGroup


@pytest.mark.importance("critical")
@pytest.mark.topology(KnownTopologyGroup.AnyProvider)
def test_sssd__selecting_profile_and_user_login(client: Client, provider: GenericProvider):
    """
    :title: Authselect sssd profile is selected and functionally tested
    :setup:
        1. Create POSIX user "user-1"
        2. Select SSSD profile
        3. Start SSSD
    :steps:
        1. Select sssd authselect profile
        2. Authenticate through SSH and then su as "user-1"
    :expectedresults:
        1. SSSD profile is selected
        2. Authentication attempts are successful for "user-1"
    :customerscenario: False
    :requirement: IDM-SSSD-TC: authselect: Authselect SSSD profile manual Client configuration
    """
    provider.user("user-1").add()

    client.authselect.select("sssd")
    client.sssd.start()

    assert client.auth.ssh.password("user-1", "Secret123")
    assert client.auth.su.password("user-1", "Secret123")


@pytest.mark.importance("critical")
@pytest.mark.topology(KnownTopologyGroup.AnyProvider)
def test_sssd__enabling_and_then_disabling_with_mkhomedir_feature(client: Client, provider: GenericProvider):
    """
    :title: Authselect sssd profile with-mkhomedir is functionally tested
    :description:
        Tests with-mkhomedir feature, with is the automatic user home directory upon login.
    :setup:
        1. Create POSIX user "user-1"
        2. Select SSSD profile with with-mkhomedir
        3. Start SSSD
    :steps:
        1. Authenticate as "user-1" and check home directory
        2. Delete "user-1" home directory and disable with-mkhomedir feature
        3. Authenticate as "user-1" and check home directory
    :expectedresults:
        1. Authentication is successful for "user-1" and home directory exists
        2. Home directory is deleted and feature is disabled
        3. Authentication is successful for "user-1" and no home directory exists
    :customerscenario: False
    :requirement: True
    """
    provider.user("user-1").add(home="/home/user-1")

    client.authselect.select("sssd", ["with-mkhomedir"])
    client.sssd.start()

    assert client.auth.ssh.password("user-1", "Secret123")
    assert client.fs.exists("/home/user-1")

    client.tools.host.ssh.run("rm -rf /home/user-1")
    client.authselect.disable_feature(["with-mkhomedir"])

    assert client.auth.ssh.password("user-1", "Secret123")
    assert not client.fs.exists("/home/user-1")


@pytest.mark.importance("critical")
@pytest.mark.topology(KnownTopologyGroup.AnyProvider)
def test_sssd__enabling_and_then_disabling_with_faillock_feature(client: Client, provider: GenericProvider):
    """
    :title: Authselect sssd profile with-faillock is functionally tested
    :description:
        PAM Faillock is a module that manages security policies on the host, login attempts, lockout duration, etc.
    :setup:
        1. Create POSIX user "user-1"
        2. Select SSSD profile with-faillock feature
        3. Start SSSD
    :steps:
        1. Authenticate as "user-1"
        2. Authenticate as "user-1" 3 times with an invalid password
        3. Reset faillock for "user-1"
        4. Authenticate as "user-1"
        5. Disable feature pam-faillock
        6. Authenticate as "user-1" 3 times with an invalid password
        7. Authenticate as "user-1"
    :expectedresults:
        1. Authentication is successful for "user-1"
        2. Authentication attempts are unsuccessful for "user-1"
        3. Faillock is reset for "user-1"
        4. Authentication is successful for "user-1"
        5. Feature pam-faillock is disabled
        6. Authentication attempts are unsuccessful for "user-1"
        7. Authentication is successful for "user-1"
    :customerscenario: False
    :requirement: IDM-SSSD-TC: authselect: Authselect SSSD profile enable and disable faillock
    """
    provider.user("user-1").add()
    faillock = client.pam.faillock()
    faillock.config_set({"deny": "3", "unlock_time": "300"})

    client.sssd.common.pam(["with-faillock"])
    client.sssd.start()

    assert client.auth.ssh.password("user-1", "Secret123")

    for i in range(3):
        client.auth.ssh.password("user-1", "BadSecret123")

    assert not client.auth.ssh.password("user-1", "Secret123")
    client.tools.faillock(["--user", "user-1", "--reset"])
    assert client.auth.ssh.password("user-1", "Secret123")

    client.authselect.disable_feature(["with-faillock"])

    for i in range(3):
        client.auth.ssh.password("user-1", "BadSecret123")
    assert client.auth.ssh.password("user-1", "Secret123")


@pytest.mark.importance("critical")
@pytest.mark.topology(KnownTopologyGroup.AnyProvider)
def test_sssd__enabling_and_then_disabling_pam_sudo_feature(client: Client, provider: GenericProvider):
    """
    :title: Authselect sssd profile with-sudo is functionally tested
    :description:
        Test enabling and disabling with-sudo.
    :setup:
        1. Create POSIX user "user-1"
        2. Add sudo rules for "user-1"
        3. Configure sudo on the client
        4. Start SSSD
    :steps:
        1. List and run sudo commands as "user-1"
        2. Disable with-sudo feature
        3. List and run sudo command as "user-1"
    :expectedresults:
        1. Sudo rule are listed and sudo command is successful
        2. Feature with-sudo is disabled
        3. Sudo rule are not listed and sudo command is unsuccessful
    :customerscenario: False
    :requirement: IDM-SSSD-TC: authselect: Authselect SSSD profile enable and disable PAM sudo
    """
    provider.user("user-1").add()
    provider.sudorule("test").add(user="user-1", host="ALL", command="/bin/ls")

    client.sssd.common.sudo()
    client.sssd.start()

    assert client.auth.sudo.list("user-1", "Secret123", expected=["(root) /bin/ls"])
    assert client.auth.sudo.run("user-1", "Secret123", command="/bin/ls /root")

    client.authselect.disable_feature(["with-sudo"])

    assert not client.auth.sudo.list("user-1", "Secret123", expected=["(root) /bin/ls"])
    assert not client.auth.sudo.run("user-1", "Secret123", command="/bin/ls /root")


@pytest.mark.importance("critical")
@pytest.mark.topology(KnownTopologyGroup.AnyProvider)
def test_sssd__enabling_and_then_disabling_pam_access_feature(client: Client, provider: GenericProvider):
    """
    :title: Authselect sssd profile with-pamaccess is functionally tested
    :description:
        PAM Access manages host based access control locally and is configured in /etc/security/access.conf
        When configured a user can be restricted from what host/session they are connection from.
    :setup:
        1. Create local users "user-1" and "user-2"
        2. Create PAM Access rules
        3. Select SSSD profile with-pamaccess feature
        4. Set "use_fully_qualified_names = False" in sssd.conf
        5. Start SSSD
    :steps:
        1. Authenticate as "user-1" then "user-2"
        2. Disable authselect feature with-pamaccess
        3. Authenticate as "user-1" then "user-2"
    :expectedresults:
        1. Authentication is successful for "user-1" and unsuccessful for "user-2"
        2. Feature with-pamaccess is disabled
        3. Authentication is successful for "user-1" and "user-2"
    :customerscenario: False
    :requirement: IDM-SSSD-TC: authselect: Authselect SSSD profile enable and disable PAM access
    """
    provider.user("user-1").add()
    provider.user("user-2").add()

    access = client.pam.access()
    access.config_set(
        [{"access": "+", "user": "user-1", "origin": "ALL"}, {"access": "-", "user": "user-2", "origin": "ALL"}]
    )

    client.sssd.authselect.enable_feature(["with-pamaccess"])
    client.sssd.domain["use_fully_qualified_names"] = "False"
    client.sssd.start()

    assert client.auth.ssh.password("user-1", "Secret123")
    assert not client.auth.ssh.password("user-2", "Secret123")

    client.authselect.disable_feature(["with-pamaccess"])

    assert client.auth.ssh.password("user-1", "Secret123")
    assert client.auth.ssh.password("user-2", "Secret123")


@pytest.mark.importance("critical")
@pytest.mark.topology(KnownTopologyGroup.AnyProvider)
def test_sssd__enabling_and_then_disabling_silent_lastlog_feature(client: Client, provider: GenericProvider):
    """
    :title: Authselect sssd profile with-silent-lastlog
    :description:
        pam_lastlog prints the "last login" date. Enabling with-silent-lastlog should disable the output. By default,
        this only works when switching users.
    :setup:
        1. Create provider user "user-1"
        2. Start SSSD with silent-last-log
        3. Start SSSD
    :steps:
        1. SU as "user-1 twice, once to update lastlog db, then su to the user and check output
        2. Disable silent-last-log
        3. SU to "user-1 and check output
    :expectedresults:
        1. Authentication attempts are successful for "user-1" and output contains no last login information
        2. Feature with-silent-lastlog is disabled
        3. Authentication is successful for "user-1" and output contains last login information
    :customerscenario: False
    :requirement: IDM-SSSD-TC: authselect: Authselect SSSD profile enable and disable PAM silent lastlog
    """
    provider.user("user-1").add()
    client.authselect.select("sssd", ["with-silent-lastlog"])
    client.sssd.start()

    client.auth.su.password_with_output("user-1", password="Secret123")
    result = client.auth.su.password_with_output("user-1", password="Secret123")
    assert "Last login:" not in result[2]

    client.authselect.disable_feature(["with-silent-lastlog"])

    result = client.auth.su.password_with_output("user-1", password="Secret123")
    assert "Last login:" in result[2]


@pytest.mark.importance("critical")
@pytest.mark.ticket(bz=2077893)
@pytest.mark.topology(KnownTopologyGroup.AnyAD)
@pytest.mark.topology(KnownTopology.IPA)
def test_sssd__enabling_and_then_disabling_with_gssapi_feature(client: Client, provider: GenericProvider):
    """
    :title: Authselect sssd profile with-gssapi feature
    :setup:
        1. Create provider user "user-1"
        2. Select SSSD profile with-gssapi
        3. Start SSSD
    :steps:
        1. kinit as user and list and run sudo commands as "user-1"
        2. Disable with-gssapi feature
        3. kinit as user and list and run sudo command as "user-1"
    :expectedresults:
        1. Sudo rule are listed and sudo command is successful
        2. Feature with-gssapi is disabled
        3. Sudo rule are not listed and sudo command is unsuccessful
    :customerscenario: False
    :requirement: IDM-SSSD-TC: sssd: bz2077893 add with-gssapi (pam_sss_gss.so)
    """
    provider.user("user-1").add()
    provider.sudorule("test").add(user="user-1", host="ALL", command="/bin/ls")
    client.sssd.common.gssapi()
    client.sssd.start()

    with client.ssh("user-1", "Secret123") as ssh:
        assert ssh.run(f"kinit user-1@{provider.host.realm}", input="Secret123")
        assert client.auth.sudo.list("user-1", expected=["(root) /bin/ls"])
        assert client.auth.sudo.run("user-1", command="/bin/ls /root")

    client.sssd.authselect.disable_feature(["with-gssapi"])

    with client.ssh("user-1", "Secret123") as ssh:
        assert ssh.run(f"kinit user-1@{provider.host.realm}", input="Secret123")
        assert not client.auth.sudo.list("user-1", expected=["(root) /bin/ls"])
        assert not client.auth.sudo.run("user-1", command="/bin/ls /root")
