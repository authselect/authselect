"""
Auuthselect Local Profile Test Cases

:requirement: authselect
"""
from __future__ import annotations

import pytest
from sssd_test_framework.roles.client import Client
from sssd_test_framework.topology import KnownTopology


@pytest.mark.skip
@pytest.mark.importance("critical")
@pytest.mark.ticket(bz=1654018)
@pytest.mark.topology(KnownTopology.Client)
def test_local__user_authentication(client: Client):
    """
    :title: Authselect local profile is functionally tested
    :setup:
        1. Create local user "user-1"
        2. Select local profile
        3. Configure SSSD for local proxy provider
        4. Start SSSD
    :steps:
        1. Authenticate as "user-1"
    :expectedresults:
        1. Authentication is successful for "user-1"
    :customerscenario: False
    :requirement: authselect local profile user authentication
    :todo: needs testing
    """
    client.local.user("user-1").add()
    client.authselect.select("local")
    client.sssd.common.local()
    client.sssd.start()

    assert client.auth.ssh.password("user-1", "Secret123")
