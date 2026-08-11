"""
Authselect Winbind Profile Test Cases

:requirement: Authselect replaced authconfig
"""

from __future__ import annotations

import pytest
from authselect_test_framework.profiles import Profile
from authselect_test_framework.roles.client import Client
from authselect_test_framework.roles.samba import Samba


@pytest.mark.importance("critical")
@pytest.mark.topology(Profile.Winbind)
def test_winbind__with_krb5(client: Client, provider: Samba):
    """
    :title: Functional authselect with-krb5 test
    :description:
        With 'with-krb5', pam_winbind krb5_auth is enabled so a domain user can
        obtain and use a Kerberos ticket in an interactive session. Disabling the
        feature does not block password login or manual kinit.
    :setup:
        1. Create a Samba AD user
    :steps:
        1. Select authselect profile with 'with-krb5' feature
        2. Open a domain user session, obtain a Kerberos ticket, and list it
        3. Disable authselect 'with-krb5' feature
        4. Obtain a Kerberos ticket again and log in with a password
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. kinit succeeds and klist shows a TGT for the domain user
        3. Authselect feature 'with-krb5' is disabled
        4. kinit still succeeds and SSH password login succeeds
    :customerscenario: True
    """
    provider.user("user-1").add(uid=10002, gid=10002, home="/home/user-1", shell="/bin/bash")

    client.authselect.select("winbind", ["with-krb5"])
    client.winbind.start()

    result = client.host.conn.run(
        f'su - "user-1" -c "kinit user-1@{provider.realm} && klist"',
        input="Secret123",
        raise_on_error=False,
    )
    assert result.rc == 0, "kinit should succeed with 'with-krb5' enabled!"
    assert f"krbtgt/{provider.realm}@{provider.realm}" in result.stdout, "klist should show a TGT!"

    client.authselect.disable_feature(["with-krb5"])

    result = client.host.conn.run(
        f'su - "user-1" -c "kinit user-1@{provider.realm} && klist"',
        input="Secret123",
        raise_on_error=False,
    )
    assert result.rc == 0, "kinit should still succeed after 'with-krb5' was disabled!"
    assert f"krbtgt/{provider.realm}@{provider.realm}" in result.stdout, "klist should still show a TGT!"
    assert client.auth.ssh.password(
        "user-1", password="Secret123"
    ), "domain user password login should succeed after 'with-krb5' was disabled!"
