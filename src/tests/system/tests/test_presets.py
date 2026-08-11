"""
Authselect Presets Tests

:requirement: Authselect replaced authconfig
"""

from __future__ import annotations

import pytest
from authselect_test_framework.profiles import Profile
from authselect_test_framework.roles.client import Client


@pytest.mark.importance("critical")
@pytest.mark.topology(Profile.Local)
def test_presets__system_default__missing(client: Client):
    """
    :title: Test `authselect is-feature-enabled`
    :setup:
        1. Remove /usr/share/authselect/authselect.conf
    :steps:
        1. Run `authselect list`
    :expectedresults:
        1. No presets are available
    :customerscenario: False
    """
    # Make sure the system default configuration is missing
    client.fs.rm("/usr/share/authselect/authselect.conf")
    result = client.host.conn.run("authselect list")
    assert "Presets" not in result.stdout
    assert "@system-default" not in result.stdout


@pytest.mark.importance("critical")
@pytest.mark.topology(Profile.Local)
def test_presets__system_default__present(client: Client):
    """
    :title: Test @system-default preset is available when configured
    :setup:
        1. Create /usr/share/authselect/authselect.conf with sssd,
           with-mkhomedir, and with-gssapi
    :steps:
        1. Run `authselect list` and verify @system-default is present
        2. Select @system-default preset
        3. Verify current configuration matches the preset
    :expectedresults:
        1. @system-default preset is listed
        2. Selection succeeds
        3. Current configuration shows "sssd with-mkhomedir with-gssapi"
    :customerscenario: False
    """
    # Make sure the system default configuration is missing
    client.fs.write(
        "/usr/share/authselect/authselect.conf",
        """
        sssd
        with-mkhomedir
        with-gssapi
        """,
    )

    result = client.host.conn.run("authselect list")
    assert "Presets" in result.stdout
    assert "@system-default" in result.stdout

    client.authselect.select("@system-default")
    result = client.host.conn.run("authselect current --raw")
    assert result.stdout == "sssd with-mkhomedir with-gssapi"


@pytest.mark.importance("critical")
@pytest.mark.topology(Profile.Local)
def test_presets__system_default__present_additional_features(client: Client):
    """
    :title: Test @system-default preset with additional features
    :setup:
        1. Create /usr/share/authselect/authselect.conf with sssd and
           with-mkhomedir
    :steps:
        1. Run `authselect list` and verify @system-default is present
        2. Select @system-default with additional feature with-gssapi
        3. Verify current configuration includes all features
    :expectedresults:
        1. @system-default preset is listed
        2. Selection succeeds
        3. Current configuration shows "sssd with-mkhomedir with-gssapi"
    :customerscenario: False
    """
    # Make sure the system default configuration is missing
    client.fs.write(
        "/usr/share/authselect/authselect.conf",
        """
        sssd
        with-mkhomedir
        """,
    )

    result = client.host.conn.run("authselect list")
    assert "Presets" in result.stdout
    assert "@system-default" in result.stdout

    client.authselect.select("@system-default", ["with-gssapi"])
    result = client.host.conn.run("authselect current --raw")
    assert result.stdout == "sssd with-mkhomedir with-gssapi"
