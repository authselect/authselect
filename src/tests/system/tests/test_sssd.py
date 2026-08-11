"""
Authselect SSSD Profile Test Cases

:requirement: Authselect replaced authconfig
"""

from __future__ import annotations

import time
from typing import cast

import pytest
from authselect_test_framework.profiles import Profile
from authselect_test_framework.roles.client import Client
from authselect_test_framework.roles.generic import GenericProvider
from authselect_test_framework.roles.ipa import IPA


@pytest.mark.importance("critical")
@pytest.mark.topology(Profile.SSSD)
def test_sssd__with_sudo(client: Client, provider: GenericProvider):
    """
    :title: Functional authselect with-sudo test
    :description:
        'with-sudo' provides centrally managed sudo rules to users on the host.
    :setup:
        1. Add sudo rule for the user
    :steps:
        1. Select authselect profile with 'with-sudo' feature
        2. List and run sudo commands as the user
        3. Disable authselect 'with-sudo' feature
        4. List and run sudo commands as the user
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. Sudo rule is listed and sudo command succeeds
        3. Authselect feature 'with-sudo' is disabled
        4. Sudo rule is not listed and sudo command fails
    :customerscenario: True
    """
    provider.user("user-1").add(home="/home/user-1", shell="/bin/bash")
    provider.sudorule("test").add(user="user-1", host="ALL", command="/bin/ls")

    client.authselect.select("sssd", ["with-sudo"])
    client.sssd.enable_responder("sudo")
    client.sssd.start()

    assert client.tools.id("user-1") is not None, "'user-1' was not found!"
    assert client.auth.sudo.list("user-1", expected=["(root) /bin/ls"]), "sudo rule should be listed!"
    assert client.auth.sudo.run("user-1", command="/bin/ls /root"), "sudo command should succeed!"

    client.authselect.disable_feature(["with-sudo"])

    assert not client.auth.sudo.list("user-1", expected=["(root) /bin/ls"]), "sudo rule should not be listed!"
    assert not client.auth.sudo.run("user-1", command="/bin/ls /root"), "sudo command should fail!"


@pytest.mark.importance("critical")
@pytest.mark.ticket(bz=2077893)
@pytest.mark.topology(Profile.SSSD)
def test_sssd__with_gssapi(client: Client, provider: GenericProvider):
    """
    :title: Functional authselect with-gssapi test
    :description:
        'with-gssapi' lets users run sudo without re-entering a password when they
        already have a valid Kerberos ticket.
    :setup:
        1. Add sudo rule for the user
    :steps:
        1. Select authselect profile with 'with-gssapi' and 'with-sudo' features
        2. Obtain Kerberos ticket and list and run sudo commands
        3. Disable authselect 'with-gssapi' feature
        4. Obtain Kerberos ticket and list and run sudo commands
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. Sudo rule is listed and sudo command succeeds
        3. Authselect feature 'with-gssapi' is disabled
        4. Sudo commands require a password
    :customerscenario: True
    """
    provider.user("user-1").add(home="/home/user-1", shell="/bin/bash")
    provider.sudorule("test").add(user="user-1", host="ALL", command="/bin/ls")

    client.authselect.select("sssd", ["with-gssapi", "with-sudo"])
    client.sssd.enable_responder("sudo")
    client.sssd.domain["pam_gssapi_services"] = "sudo, sudo-i"
    client.sssd.domain["pam_gssapi_check_upn"] = "False"
    client.sssd.start()
    time.sleep(2)

    assert client.tools.id("user-1") is not None, "'user-1' was not found!"

    result = client.host.conn.run(
        f'su - "user-1" -c "kinit user-1@{provider.realm} && sudo -l && sudo /bin/ls /root"',
        input="Secret123",
        raise_on_error=False,
    )
    assert (
        result.rc == 0
    ), f"kinit and sudo should succeed with 'with-gssapi' enabled!\n{result.stdout}\n{result.stderr}"
    assert "(root) /bin/ls" in result.stdout, "sudo rule should be listed!"

    client.authselect.disable_feature(["with-gssapi"])

    result = client.host.conn.run(
        f'su - "user-1" -c "kinit user-1@{provider.realm} && sudo -l"',
        input="Secret123",
        raise_on_error=False,
    )
    assert result.rc != 0, "sudo -l should fail after 'with-gssapi' was disabled!"
    assert "sudo: a password is required" in result.stderr, "sudo -l should require a password!"

    result = client.host.conn.run(
        f'su - "user-1" -c "kinit user-1@{provider.realm} && sudo /bin/ls /root"',
        input="Secret123",
        raise_on_error=False,
    )
    assert result.rc != 0, "sudo command should fail after 'with-gssapi' was disabled!"
    assert "sudo: a password is required" in result.stderr, "sudo command should require a password!"


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.SSSD)
def test_sssd__with_files_access_provider(client: Client, provider: GenericProvider):
    """
    :title: Functional authselect with-files-access-provider test
    :description:
        'with-files-access-provider' lets local users log in when the central
        identity service is used for other accounts.
    :setup:
    :steps:
        1. Select authselect profile with 'with-files-access-provider' feature
        2. Login as the user
        3. Disable authselect 'with-files-access-provider' feature
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. Login is successful
        3. Authselect feature 'with-files-access-provider' is disabled
    :customerscenario: False
    """
    client.user("user-2").add(uid=10002, gid=10002, home="/home/user-2", shell="/bin/bash")

    client.authselect.select("sssd", ["with-files-access-provider"])
    client.sssd.start()

    assert client.tools.id("user-2") is not None, "'user-2' was not found!"
    assert client.auth.ssh.password("user-2", password="Secret123"), "SSH authentication should succeed!"

    client.authselect.disable_feature(["with-files-access-provider"])


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.SSSD)
def test_sssd__with_smartcard(client: Client, provider: GenericProvider):
    """
    :title: Functional authselect with-smartcard test
    :description:
        'with-smartcard' allows login using a smart card certificate.
    :setup:
        1. Enroll certificate on token
    :steps:
        1. Select authselect profile with 'with-smartcard' feature
        2. Verify authselect-generated PAM configuration
        3. Authenticate as the IPA user via nested ``su`` with the smart card PIN
        4. Disable authselect 'with-smartcard' feature
        5. Attempt to authenticate via nested ``su`` with the smart card PIN again
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. system-auth includes pam_sss try_cert_auth
        3. PIN prompt appears and authentication succeeds
        4. Authselect feature 'with-smartcard' is disabled
        5. Smart card authentication does not succeed
    :customerscenario: False
    """
    ipa = cast(IPA, provider)
    provider.user("user-1").add(home="/home/user-1", shell="/bin/bash")

    client.smartcard.enroll_to_token(client, ipa, "user-1", pin="123456", init=True)
    client.sssd.common.smartcard_with_softhsm(client.smartcard)

    assert client.tools.id("user-1") is not None, "'user-1' was not found!"
    system_auth = client.fs.read("/etc/pam.d/system-auth")
    assert (
        "pam_sss.so" in system_auth and "try_cert_auth" in system_auth
    ), "system-auth should include pam_sss try_cert_auth!"

    assert client.auth.su.smartcard("user-1", "123456"), "Smart card authentication should succeed!"

    client.authselect.disable_feature(["with-smartcard"])

    system_auth = client.fs.read("/etc/pam.d/system-auth")
    assert "try_cert_auth" not in system_auth, "system-auth should not include pam_sss try_cert_auth!"

    # Authselect only manages PAM; revert the SSSD settings from REQUIREMENTS as an
    # admin would when turning smart card authentication off.
    del client.sssd.pam["pam_cert_auth"]
    del client.sssd.domain["local_auth_policy"]
    client.sssd.restart(clean=True)

    assert not client.auth.su.smartcard(
        "user-1", "123456"
    ), "Smart card authentication should fail after 'with-smartcard' was disabled!"


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.SSSD)
def test_sssd__with_smartcard_lock_on_removal(client: Client):
    """
    :title: Sanity authselect with-smartcard-lock-on-removal test
    :description:
        'with-smartcard-lock-on-removal' locks the session when the smart card is
        removed.
    :setup:
    :steps:
        1. Select authselect profile with 'with-smartcard' and 'with-smartcard-lock-on-removal' features
        2. Disable authselect 'with-smartcard-lock-on-removal' feature
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. Authselect feature 'with-smartcard-lock-on-removal' is disabled
    :customerscenario: False
    """
    client.authselect.select("sssd", ["with-smartcard", "with-smartcard-lock-on-removal"])

    client.authselect.disable_feature(["with-smartcard-lock-on-removal"])


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.SSSD)
def test_sssd__with_smartcard_required(client: Client):
    """
    :title: Sanity authselect with-smartcard-required test
    :description:
        'with-smartcard-required' requires smart card login; password-only login is
        not allowed.
    :setup:
    :steps:
        1. Select authselect profile with 'with-smartcard-required' feature
        2. Verify authselect-generated PAM configuration
        3. Disable authselect 'with-smartcard-required' feature
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. system-auth includes require_cert_auth
        3. Authselect feature 'with-smartcard-required' is disabled
    :customerscenario: False
    """
    client.authselect.select("sssd", ["with-smartcard-required"])

    system_auth = client.fs.read("/etc/pam.d/system-auth")
    assert "require_cert_auth" in system_auth, "system-auth should include require_cert_auth!"

    client.authselect.disable_feature(["with-smartcard-required"])


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.SSSD)
def test_sssd__with_subid(client: Client, provider: GenericProvider):
    """
    :title: Functional authselect with-subid test
    :description:
        'with-subid' provides subordinate user and group ID ranges from the central
        directory.
    :setup:
        1. Configure subid ranges for the user
    :steps:
        1. Select authselect profile with 'with-subid' feature
        2. Lookup subid ranges
        3. Disable authselect 'with-subid' feature
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. Subid ranges are returned for the user
        3. Authselect feature 'with-subid' is disabled
    :customerscenario: False
    """
    ipa = cast(IPA, provider)
    provider.user("user-3").add(home="/home/user-3", shell="/bin/bash")
    subid = ipa.user("user-3").subid().generate()

    client.authselect.select("sssd", ["with-subid"])
    client.sssd.start()

    assert client.tools.id("user-3") is not None, "'user-3' was not found!"

    nsswitch = client.fs.read("/etc/nsswitch.conf")
    subid_line = next(line for line in nsswitch.splitlines() if line.startswith("subid:"))
    assert "sss" in subid_line, "subid nsswitch entry should include sss!"

    entry = client.tools.getsubid("user-3")
    assert entry is not None, "getsubids should return subid ranges!"
    assert entry.range_start == subid.uid_start, "SubUID range start should match IPA subid entry!"
    assert entry.range_size == subid.uid_size, "SubUID range size should match IPA subid entry!"

    client.authselect.disable_feature(["with-subid"])


@pytest.mark.importance("high")
@pytest.mark.ticket(jira="RHEL-181749")
@pytest.mark.topology(Profile.SSSD)
def test_sssd__with_switchable_auth(client: Client):
    """
    :title: Sanity authselect with-switchable-auth test
    :description:
        'with-switchable-auth' is the authselect side of the passwordless-gdm
        feature. It enables the 'switchable-auth' PAM service, used by login
        applications such as GDM to offer the user a choice of authentication
        methods, and flips the 'enable-switchable-authentication' dconf key
        that GDM reads to decide whether to offer passwordless login at all.
        Without the feature the PAM service is a stub that blocks all
        authentication and the dconf key stays disabled.
    :setup:
    :steps:
        1. Select authselect profile with 'with-switchable-auth' feature
        2. Verify the 'switchable-auth' PAM service contains the full auth stack
        3. Verify the 'enable-switchable-authentication' dconf key is enabled
        4. Disable authselect 'with-switchable-auth' feature
        5. Verify the 'switchable-auth' PAM service no longer has an auth stack
        6. Verify the 'enable-switchable-authentication' dconf key is disabled
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. 'switchable-auth' contains 'pam_unix.so' and 'pam_sss.so'
        3. dconf reports 'enable-switchable-authentication=true'
        4. Authselect feature 'with-switchable-auth' is disabled
        5. 'switchable-auth' does not contain 'pam_unix.so' or 'pam_sss.so'
        6. dconf reports 'enable-switchable-authentication=false'
    :customerscenario: False
    """
    client.authselect.select("sssd", ["with-switchable-auth"])

    switchable_auth = client.fs.read("/etc/pam.d/switchable-auth")
    assert "pam_unix.so" in switchable_auth, "'switchable-auth' should contain 'pam_unix.so'!"
    assert "pam_sss.so" in switchable_auth, "'switchable-auth' should contain 'pam_sss.so'!"

    dconf_db = client.fs.read("/etc/authselect/dconf-db")
    assert (
        "enable-switchable-authentication=true" in dconf_db
    ), "GDM passwordless login should be advertised via dconf when 'with-switchable-auth' is enabled!"

    client.authselect.disable_feature(["with-switchable-auth"])

    switchable_auth = client.fs.read("/etc/pam.d/switchable-auth")
    assert (
        "pam_unix.so" not in switchable_auth
    ), "'switchable-auth' should not contain 'pam_unix.so' when feature is disabled!"
    assert (
        "pam_sss.so" not in switchable_auth
    ), "'switchable-auth' should not contain 'pam_sss.so' when feature is disabled!"

    dconf_db = client.fs.read("/etc/authselect/dconf-db")
    assert (
        "enable-switchable-authentication=false" in dconf_db
    ), "GDM passwordless login should not be advertised via dconf when 'with-switchable-auth' is disabled!"


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.SSSD)
def test_sssd__with_gpupdate(client: Client):
    """
    :title: Sanity authselect with-gpupdate test
    :description:
        Verify 'with-gpupdate' adds 'pam_oddjob_gpupdate.so' to the PAM session
        stack so Group Policy Objects are applied automatically on user login.
    :setup:
    :steps:
        1. Select authselect profile with 'with-gpupdate' feature
        2. Verify the PAM session stack contains 'pam_oddjob_gpupdate.so'
        3. Disable authselect 'with-gpupdate' feature
        4. Verify the PAM session stack no longer contains 'pam_oddjob_gpupdate.so'
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. 'system-auth' contains 'pam_oddjob_gpupdate.so'
        3. Authselect feature 'with-gpupdate' is disabled
        4. 'system-auth' does not contain 'pam_oddjob_gpupdate.so'
    :customerscenario: False
    """
    client.authselect.select("sssd", ["with-gpupdate"])

    system_auth = client.fs.read("/etc/pam.d/system-auth")
    assert (
        "pam_oddjob_gpupdate.so" in system_auth
    ), "'system-auth' should contain 'pam_oddjob_gpupdate.so' when 'with-gpupdate' is enabled!"

    client.authselect.disable_feature(["with-gpupdate"])

    system_auth = client.fs.read("/etc/pam.d/system-auth")
    assert (
        "pam_oddjob_gpupdate.so" not in system_auth
    ), "'system-auth' should not contain 'pam_oddjob_gpupdate.so' when 'with-gpupdate' is disabled!"


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.SSSD)
def test_sssd__with_tlog(client: Client):
    """
    :title: Sanity authselect with-tlog test
    :description:
        'with-tlog' integrates user and group lookups with terminal session recording.
    :setup:
    :steps:
        1. Select authselect profile with 'with-tlog' feature
        2. Verify authselect-generated nsswitch configuration
        3. Disable authselect 'with-tlog' feature
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. nsswitch passwd and group entries start with sss
        3. Authselect feature 'with-tlog' is disabled
    :customerscenario: False
    """
    client.authselect.select("sssd", ["with-tlog"])

    nsswitch = client.fs.read("/etc/nsswitch.conf")
    passwd_line = next(line for line in nsswitch.splitlines() if line.startswith("passwd:"))
    group_line = next(line for line in nsswitch.splitlines() if line.startswith("group:"))
    assert (
        passwd_line.startswith("passwd:") and "sss" in passwd_line.split()[1]
    ), "passwd nsswitch entry should start with sss!"
    assert (
        group_line.startswith("group:") and "sss" in group_line.split()[1]
    ), "group nsswitch entry should start with sss!"

    client.authselect.disable_feature(["with-tlog"])


@pytest.mark.importance("critical")
@pytest.mark.ticket(jira="SSSD-7707")
@pytest.mark.topology(Profile.SSSD)
def test_sssd__with_group_merging(client: Client, provider: GenericProvider):
    """
    :title: Functional authselect with-group-merging test
    :description:
        'with-group-merging' merges group membership from local files and the directory
        so a user appears in a group even when only the directory records the membership.
    :setup:
        1. Create a provide group and user and add the provider user as a member
        2. Create a local group and user
        3. Add both provider and local users as members, using sed
        4. Start the SSSD service
    :steps:
        1. Select authselect profile with 'with-group-merging' feature
        2. Lookup users and group
        3. Disable authselect 'with-group-merging' feature
        4. Lookup local group
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. Group is found with all users as members
        3. Authselect feature 'with-group-merging' is disabled
        4. Group is found with just the provider user as a member
    :customerscenario: False
    """
    provider.group("group0").add(gid=100000).add_member(provider.user("user0").add())
    client.user("user1").add()
    client.group("group0").add(gid=100000)

    # A sed is used to skip user checks
    client.fs.backup("/etc/group")
    client.fs.sed("/^group0:/ { s/$/,user1,user2/; s/:,/:/g }", "/etc/group", args=["-i"])

    client.sssd.start()

    client.authselect.select(client.profile, ["with-group-merging"])

    assert client.tools.getent.passwd("user0") is not None, "'user0' was not found!"
    assert client.tools.getent.passwd("user1") is not None, "'user1' was not found!"
    assert client.tools.getent.group("group0") is not None, "'group0' was not found!"

    result = client.tools.getent.group("group0")
    assert result is not None, "'group0' was not found!"
    assert "user0" in result.members, "'user0' was not a member of 'group0'!"
    assert "user1" in result.members, "'user1' was not a member of 'group0'!"

    client.authselect.disable_feature(["with-group-merging"])

    result = client.tools.getent.group("group0")
    assert result is not None, "'group0' was not found!"
    assert "user0" not in result.members, "'user0' was a member of 'group0'!"
    assert "user1" in result.members, "'user1' was not a member of 'group0'!"
