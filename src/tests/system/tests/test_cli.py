"""
Authselect Command Line Interface Tests

:requirement: Authselect replaced authconfig
"""

from __future__ import annotations

import pytest
from sssd_test_framework.roles.client import Client
from sssd_test_framework.topology import KnownTopology


@pytest.mark.importance("critical")
@pytest.mark.topology(KnownTopology.Client)
def test_cli__is_feature_enabled(client: Client):
    """
    :title: Test `authselect is-feature-enabled`
    :setup:
        1. Select local profile with with-mkhomedir feature
    :steps:
        1. Run `authselect is-feature-enabled with-mkhomedir`
        2. Run `authselect is-feature-enabled with-fingerprint`
    :expectedresults:
        1. Return 0, empty output
        2. Return 2, not empty output
    :customerscenario: False
    """
    client.authselect.select("local", ["with-mkhomedir"])

    result = client.host.conn.run("authselect is-feature-enabled with-mkhomedir", raise_on_error=False)
    assert result.rc == 0
    assert not result.stdout
    assert not result.stderr

    result = client.host.conn.run("authselect is-feature-enabled with-fingerprint", raise_on_error=False)
    assert result.rc == 2
    assert not result.stdout
    assert not result.stderr
