"""
Miscellaneous Tests
"""

from __future__ import annotations

import pytest
from sssd_test_framework.roles.client import Client
from sssd_test_framework.topology import KnownTopology


@pytest.mark.importance("low")
@pytest.mark.ticket(jira=39537)
@pytest.mark.parametrize("profile", ["local", "nis", "sssd", "winbind"])
@pytest.mark.topology(KnownTopology.Client)
def test_misc__default_profile_nsswitch_conf_can_resolve_client_hostname(client: Client, profile: str):
    """
    :title: Applying authselect profiles does not break client hostname resolution
    :setup:
        1: Select and apply authselect profile
    :steps:
        1. Lookup client hostname using nslookup
    :expectedresults:
        1. Resolves the client hostname
    :customerscenario: True
    """
    client.authselect.select(profile)

    assert client.fs.exists("/etc/hostname"), "Missing /etc/hostname file!"
    hostname = client.fs.read("/etc/hostname")

    assert hostname is not None, "Hostname not found!"
    assert client.host.conn.exec(["nslookup", hostname]).rc == 0, "Cannot resolve hostname!"
