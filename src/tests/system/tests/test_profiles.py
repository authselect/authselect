"""
Authselect tests shared across profiles.

:requirement: Authselect replaced authconfig
"""

from __future__ import annotations

import pytest
from authselect_test_framework.profiles import Profile, ProfileGroup
from authselect_test_framework.roles.client import Client
from authselect_test_framework.roles.generic import GenericProvider
from authselect_test_framework.utils.pam import PAMAccessUtils, PAMFaillockUtils
from pytest_mh import mh_utility


def start_identity_service(client: Client) -> None:
    if client.profile is Profile.SSSD:
        client.sssd.enable_responder("pam")
        client.sssd.start()
    elif client.profile is Profile.Winbind:
        client.winbind.start()


@pytest.mark.importance("critical")
@pytest.mark.topology(ProfileGroup.AnyProfile)
def test_profiles__with_faillock(
    client: Client,
    provider: GenericProvider,
):
    """
    :title: Functional authselect with-faillock test
    :description:
        'with-faillock' limits failed login attempts and locks the account after
        too many wrong passwords.
    :setup:
        1. Configure and edit faillock in /etc/security/faillock.conf
        2. Start the identity service
    :steps:
        1. Select authselect profile with 'with-faillock' feature
        2. Login with the correct password, then login three times with a bad password
        3. Disable authselect 'with-faillock' feature
        4. Login with the correct password
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. First login is successful, remaining are unsuccessful
        3. Authselect feature 'with-faillock' is disabled
        4. Login is successful
    :customerscenario: True
    """
    provider.user("user1").add(home="/home/user1", shell="/bin/bash")

    start_identity_service(client)

    faillock = PAMFaillockUtils(client.host, client.fs)
    with mh_utility(faillock):
        faillock.config_set({"deny": "3", "unlock_time": "300"})

        client.authselect.select(client.profile, ["with-faillock"])

        faillock.reset_user("user1")

        if client.profile is not Profile.Local:
            assert client.tools.id("user1") is not None, "'user1' was not found!"
        assert client.auth.su.password("user1", password="Secret123"), "'user1' was unable to log in via su!"

        for _ in range(3):
            client.auth.su.password("user1", password="BadSecret123")

        assert not client.auth.su.password(
            "user1", password="Secret123"
        ), "'user1' was not locked out after failed attempts!"

        client.authselect.disable_feature(["with-faillock"])

        assert client.auth.su.password(
            "user1", password="Secret123"
        ), "'user1' was unable to log in via su after 'with-faillock' was disabled!"


@pytest.mark.importance("critical")
@pytest.mark.topology(ProfileGroup.AnyProfile)
def test_profiles__with_mkhomedir(
    client: Client,
    provider: GenericProvider,
):
    """
    :title: Functional authselect with-mkhomedir test
    :description:
        'with-mkhomedir' creates the user home directory on first login.
    :setup:
        1. Start the identity service
    :steps:
        1. Select authselect profile with 'with-mkhomedir' feature
        2. Login and verify home directory is created
        3. Disable authselect 'with-mkhomedir' feature
        4. Login and verify home directory is not created
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. Login is successful and home directory exists
        3. Authselect feature 'with-mkhomedir' is disabled
        4. Login is successful and home directory does not exist
    :customerscenario: True
    """
    home = "/home/user1"
    provider.user("user1").add(home="/home/user1", shell="/bin/bash")

    start_identity_service(client)

    client.authselect.select(client.profile, ["with-mkhomedir"])

    if client.profile is Profile.Winbind:
        passwd = client.tools.getent.passwd("user1")
        assert passwd is not None, "'user1' was not found!"
        assert passwd.home is not None, "'user1' home was not set in passwd!"
        home = passwd.home

    client.oddjob.start()

    assert client.tools.getent.passwd("user1") is not None, "'user1' was not found!"

    assert client.auth.ssh.password("user1", password="Secret123"), "'user1' was unable to log in!"
    assert client.fs.exists(home), "home directory was not found!"

    client.fs.rm(home)
    client.authselect.disable_feature(["with-mkhomedir"])

    assert client.auth.ssh.password("user1", password="Secret123"), "'user1' was unable to log in!"
    assert not client.fs.exists(home), "home directory was found!"


@pytest.mark.importance("critical")
@pytest.mark.topology(ProfileGroup.AnyProfile)
def test_profiles__with_pamaccess(
    client: Client,
    provider: GenericProvider,
):
    """
    :title: Functional authselect with-pamaccess test
    :description:
        'with-pamaccess' controls which users can log in from which locations.
    :setup:
        1. Configure and edit /etc/security/access.conf
        2. Start the identity service
    :steps:
        1. Select authselect profile with 'with-pamaccess' feature
        2. Login as permitted and denied users
        3. Disable authselect 'with-pamaccess' feature
        4. Login as both users again
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. Permitted user login is successful, denied user login is unsuccessful
        3. Authselect feature 'with-pamaccess' is disabled
        4. Both user logins are successful
    :customerscenario: True
    """
    provider.user("user1").add(home="/home/user1", shell="/bin/bash")
    provider.user("user-2").add(home="/home/user-2", shell="/bin/bash")

    access = PAMAccessUtils(client.host, client.fs)
    with mh_utility(access):
        access.config_set(
            [{"access": "+", "user": "user1", "origin": "ALL"}, {"access": "-", "user": "user-2", "origin": "ALL"}]
        )

        start_identity_service(client)

        client.authselect.select(client.profile, ["with-pamaccess"])

        if client.profile is not Profile.Local:
            assert client.tools.getent.passwd("user1") is not None, "'user1' was not found!"
            assert client.tools.getent.passwd("user-2") is not None, "'user-2' was not found!"

        assert client.auth.ssh.password("user1", password="Secret123"), "'user1' was unable to log in via ssh!"
        assert not client.auth.ssh.password("user-2", password="Secret123"), "'user-2' was able to log in via ssh!"

        client.authselect.disable_feature(["with-pamaccess"])

        assert client.auth.ssh.password("user1", password="Secret123"), "'user1' was unable to log in via ssh!"
        assert client.auth.ssh.password("user-2", password="Secret123"), "'user-2' was unable to log in via ssh!"


@pytest.mark.importance("critical")
@pytest.mark.topology(ProfileGroup.AnyProfile)
def test_profiles__with_silent_lastlog(
    client: Client,
    provider: GenericProvider,
):
    """
    :title: Functional authselect with-silent-lastlog test
    :description:
        'with-silent-lastlog' hides the last login message shown at login.
    :setup:
        1. Start the identity service
    :steps:
        1. Select authselect profile with 'with-silent-lastlog' feature
        2. Login twice and check login output
        3. Disable authselect 'with-silent-lastlog' feature
        4. Login and check login output
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. Output does not contain last login information
        3. Authselect feature 'with-silent-lastlog' is disabled
        4. Output contains last login information
    :customerscenario: True
    """
    provider.user("user1").add(home="/home/user1", shell="/bin/bash")

    start_identity_service(client)

    client.authselect.select(client.profile, ["with-silent-lastlog"])

    assert client.tools.getent.passwd("user1") is not None, "'user1' was not found!"
    if client.profile is Profile.Winbind:
        client.fs.mkdir_p("/home/user1")

    client.auth.su.password_with_output("user1", password="Secret123")
    result = client.auth.su.password_with_output("user1", password="Secret123")
    assert "Last login:" not in result[2], "'Last login:' was found in su output!"

    client.authselect.disable_feature(["with-silent-lastlog"])

    result = client.auth.su.password_with_output("user1", password="Secret123")
    assert "Last login:" in result[2], "'Last login:' was not found in su output!"


@pytest.mark.importance("high")
@pytest.mark.topology(ProfileGroup.AnyProfile)
def test_profiles__with_altfiles(client: Client):
    """
    :title: Sanity authselect with-altfiles test
    :description:
        'with-altfiles' adds 'altfiles' to the 'passwd' and 'group' nsswitch
        entries so that '/etc/alt-passwd' and '/etc/alt-group' are consulted
        for name lookups alongside the default sources.
    :setup:
    :steps:
        1. Select authselect profile with 'with-altfiles' feature
        2. Verify 'altfiles' appears in 'passwd' and 'group' nsswitch entries
        3. Disable authselect 'with-altfiles' feature
        4. Verify 'altfiles' is removed from 'passwd' and 'group' nsswitch entries
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. '/etc/nsswitch.conf' passwd and group lines contain 'altfiles'
        3. Authselect feature 'with-altfiles' is disabled
        4. '/etc/nsswitch.conf' passwd and group lines do not contain 'altfiles'
    :customerscenario: False
    """
    client.authselect.select(client.profile, ["with-altfiles"])

    nsswitch = client.fs.read("/etc/nsswitch.conf")
    passwd_line = next(line for line in nsswitch.splitlines() if line.startswith("passwd:"))
    group_line = next(line for line in nsswitch.splitlines() if line.startswith("group:"))
    assert "altfiles" in passwd_line, "'altfiles' was not found in passwd nsswitch entry!"
    assert "altfiles" in group_line, "'altfiles' was not found in group nsswitch entry!"

    client.authselect.disable_feature(["with-altfiles"])

    nsswitch = client.fs.read("/etc/nsswitch.conf")
    passwd_line = next(line for line in nsswitch.splitlines() if line.startswith("passwd:"))
    group_line = next(line for line in nsswitch.splitlines() if line.startswith("group:"))
    assert "altfiles" not in passwd_line, "'altfiles' was found in passwd nsswitch entry!"
    assert "altfiles" not in group_line, "'altfiles' was found in group nsswitch entry!"


@pytest.mark.importance("high")
@pytest.mark.topology(ProfileGroup.AnyProfile)
def test_profiles__with_fingerprint(client: Client):
    """
    :title: Sanity authselect with-fingerprint test
    :description:
        'with-fingerprint' allows login using a fingerprint reader.
    :setup:
    :steps:
        1. Select authselect profile with 'with-fingerprint' feature
        2. Verify authselect-generated PAM configuration
        3. Disable authselect 'with-fingerprint' feature
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. system-auth includes pam_fprintd.so
        3. Authselect feature 'with-fingerprint' is disabled
    :customerscenario: False
    """
    client.authselect.select(client.profile, ["with-fingerprint"])

    system_auth = client.fs.read("/etc/pam.d/system-auth")
    assert "pam_fprintd.so" in system_auth, "'pam_fprintd.so' was not found in system-auth!"

    client.authselect.disable_feature(["with-fingerprint"])


@pytest.mark.importance("high")
@pytest.mark.topology(ProfileGroup.AnyProfile)
def test_profiles__with_libvirt(client: Client):
    """
    :title: Sanity authselect with-libvirt test
    :description:
        'with-libvirt' resolves host names on libvirt virtual networks.
    :setup:
    :steps:
        1. Select authselect profile with 'with-libvirt' feature
        2. Verify authselect-generated nsswitch configuration
        3. Disable authselect 'with-libvirt' feature
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. hosts nsswitch entry includes libvirt
        3. Authselect feature 'with-libvirt' is disabled
    :customerscenario: False
    """
    client.authselect.select(client.profile, ["with-libvirt"])

    nsswitch = client.fs.read("/etc/nsswitch.conf")
    hosts_line = next(line for line in nsswitch.splitlines() if line.startswith("hosts:"))
    assert "libvirt" in hosts_line, "'libvirt' was not found in hosts nsswitch entry!"

    client.authselect.disable_feature(["with-libvirt"])


@pytest.mark.importance("high")
@pytest.mark.topology(ProfileGroup.AnyProfile)
def test_profiles__with_mdns4(client: Client):
    """
    :title: Sanity authselect with-mdns4 test
    :description:
        'with-mdns4' discovers nearby hosts on the local IPv4 network by name.
    :setup:
    :steps:
        1. Select authselect profile with 'with-mdns4' feature
        2. Verify authselect-generated nsswitch configuration
        3. Disable authselect 'with-mdns4' feature
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. hosts nsswitch entry includes mdns4_minimal
        3. Authselect feature 'with-mdns4' is disabled
    :customerscenario: False
    """
    client.authselect.select(client.profile, ["with-mdns4"])

    nsswitch = client.fs.read("/etc/nsswitch.conf")
    hosts_line = next(line for line in nsswitch.splitlines() if line.startswith("hosts:"))
    assert "mdns4_minimal" in hosts_line, "'mdns4_minimal' was not found in hosts nsswitch entry!"

    client.authselect.disable_feature(["with-mdns4"])


@pytest.mark.importance("high")
@pytest.mark.topology(ProfileGroup.AnyProfile)
def test_profiles__with_mdns6(client: Client):
    """
    :title: Sanity authselect with-mdns6 test
    :description:
        'with-mdns6' discovers nearby hosts on the local IPv6 network by name.
    :setup:
    :steps:
        1. Select authselect profile with 'with-mdns6' feature
        2. Verify authselect-generated nsswitch configuration
        3. Disable authselect 'with-mdns6' feature
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. hosts nsswitch entry includes mdns6_minimal
        3. Authselect feature 'with-mdns6' is disabled
    :customerscenario: False
    """
    client.authselect.select(client.profile, ["with-mdns6"])

    nsswitch = client.fs.read("/etc/nsswitch.conf")
    hosts_line = next(line for line in nsswitch.splitlines() if line.startswith("hosts:"))
    assert "mdns6_minimal" in hosts_line, "'mdns6_minimal' was not found in hosts nsswitch entry!"

    client.authselect.disable_feature(["with-mdns6"])


@pytest.mark.importance("high")
@pytest.mark.topology(ProfileGroup.AnyProfile)
def test_profiles__with_pam_gnome_keyring(client: Client):
    """
    :title: Sanity authselect with-pam-gnome-keyring test
    :description:
        'with-pam-gnome-keyring' unlocks saved passwords in the GNOME keyring at login.
    :setup:
    :steps:
        1. Select authselect profile with 'with-pam-gnome-keyring' feature
        2. Verify authselect-generated PAM configuration
        3. Disable authselect 'with-pam-gnome-keyring' feature
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. system-auth includes pam_gnome_keyring.so
        3. Authselect feature 'with-pam-gnome-keyring' is disabled
    :customerscenario: False
    """
    client.authselect.select(client.profile, ["with-pam-gnome-keyring"])

    system_auth = client.fs.read("/etc/pam.d/system-auth")
    assert "pam_gnome_keyring.so" in system_auth, "'pam_gnome_keyring.so' was not found in system-auth!"

    client.authselect.disable_feature(["with-pam-gnome-keyring"])


@pytest.mark.importance("high")
@pytest.mark.topology(ProfileGroup.AnyProfile)
def test_profiles__with_pam_u2f(client: Client):
    """
    :title: Sanity authselect with-pam-u2f test
    :description:
        'with-pam-u2f' allows login using a hardware security key.
    :setup:
    :steps:
        1. Select authselect profile with 'with-pam-u2f' feature
        2. Verify authselect-generated PAM configuration
        3. Disable authselect 'with-pam-u2f' feature
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. system-auth includes pam_u2f.so
        3. Authselect feature 'with-pam-u2f' is disabled
    :customerscenario: False
    """
    client.authselect.select(client.profile, ["with-pam-u2f"])

    system_auth = client.fs.read("/etc/pam.d/system-auth")
    assert "pam_u2f.so" in system_auth, "'pam_u2f.so' was not found in system-auth!"

    client.authselect.disable_feature(["with-pam-u2f"])


@pytest.mark.importance("high")
@pytest.mark.topology(ProfileGroup.AnyProfile)
def test_profiles__with_pam_u2f_2fa(client: Client):
    """
    :title: Sanity authselect with-pam-u2f-2fa test
    :description:
        'with-pam-u2f-2fa' requires a hardware security key in addition to a password.
    :setup:
    :steps:
        1. Select authselect profile with 'with-pam-u2f-2fa' feature
        2. Verify authselect-generated PAM configuration
        3. Disable authselect 'with-pam-u2f-2fa' feature
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. system-auth includes pam_u2f.so
        3. Authselect feature 'with-pam-u2f-2fa' is disabled
    :customerscenario: False
    """
    client.authselect.select(client.profile, ["with-pam-u2f-2fa"])

    system_auth = client.fs.read("/etc/pam.d/system-auth")
    assert "pam_u2f.so" in system_auth, "'pam_u2f.so' was not found in system-auth!"

    client.authselect.disable_feature(["with-pam-u2f-2fa"])


@pytest.mark.importance("high")
@pytest.mark.topology(ProfileGroup.AnyProfile)
def test_profiles__with_pwhistory(
    client: Client,
    provider: GenericProvider,
):
    """
    :title: Sanity authselect with-pwhistory test
    :description:
        'with-pwhistory' prevents reuse of recent passwords when changing a password.
    :setup:
        1. Start the identity service
    :steps:
        1. Select authselect profile with 'with-pwhistory' feature
        2. Verify authselect-generated PAM configuration
        3. Disable authselect 'with-pwhistory' feature
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. system-auth includes pam_pwhistory.so
        3. Authselect feature 'with-pwhistory' is disabled
    :customerscenario: False
    """
    provider.user("user-3").add(home="/home/user-3", shell="/bin/bash")

    start_identity_service(client)

    client.authselect.select(client.profile, ["with-pwhistory"])

    assert client.tools.getent.passwd("user-3") is not None, "'user-3' was not found!"
    system_auth = client.fs.read("/etc/pam.d/system-auth")
    assert "pam_pwhistory.so" in system_auth, "'pam_pwhistory.so' was not found in system-auth!"

    client.authselect.disable_feature(["with-pwhistory"])


@pytest.mark.importance("high")
@pytest.mark.topology(ProfileGroup.AnyProfile)
def test_profiles__without_pam_u2f_nouserok(client: Client):
    """
    :title: Sanity authselect without-pam-u2f-nouserok test
    :description:
        'without-pam-u2f-nouserok' requires a registered security key; password-only
        login is not allowed.
    :setup:
    :steps:
        1. Select authselect profile with 'with-pam-u2f-2fa' and 'without-pam-u2f-nouserok' features
        2. Verify authselect-generated PAM configuration
        3. Disable authselect 'without-pam-u2f-nouserok' feature
    :expectedresults:
        1. Authselect profile is selected with features enabled
        2. pam_u2f.so is included without nouserok
        3. Authselect feature 'without-pam-u2f-nouserok' is disabled
    :customerscenario: False
    """
    client.authselect.select(client.profile, ["with-pam-u2f-2fa", "without-pam-u2f-nouserok"])

    system_auth = client.fs.read("/etc/pam.d/system-auth")
    assert "pam_u2f.so" in system_auth, "'pam_u2f.so' was not found in system-auth!"
    assert (
        "nouserok" not in system_auth.split("pam_u2f.so")[1].split("\n")[0]
    ), "'nouserok' was found in system-auth pam_u2f entry!"

    client.authselect.disable_feature(["without-pam-u2f-nouserok"])
