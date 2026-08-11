"""
Authselect Local Profile Test Cases

:requirement: Authselect replaced authconfig
"""

from __future__ import annotations

import pytest
from authselect_test_framework.profiles import Profile
from authselect_test_framework.roles.client import Client


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.Local)
def test_local__without_nullok(client: Client):
    """
    :title: Functional authselect without-nullok test
    :description:
        'without-nullok' blocks login with a blank password by removing the
        'nullok' option from 'pam_unix.so'. Tested on the local profile because
        on sssd/winbind profiles 'pam_sss.so' short-circuits auth for local users
        before 'pam_unix.so' is reached.
    :setup:
        1. Create a local user and remove the password to leave it blank
    :steps:
        1. Select authselect profile with 'without-nullok' feature
        2. Attempt login with a blank password
        3. Disable authselect 'without-nullok' feature
        4. Verify PAM configuration restores nullok
    :expectedresults:
        1. Authselect profile is selected with feature enabled
        2. Login is denied
        3. Authselect feature 'without-nullok' is disabled
        4. 'pam_unix.so' includes 'nullok' in system-auth
    :customerscenario: False
    """
    client.user("user-3").add(uid=10003, gid=10003, home="/home/user-3", shell="/bin/bash", password="")
    client.host.conn.run("passwd -d user-3")

    client.authselect.select("local", ["without-nullok"])

    assert not client.auth.su.password(
        "user-3", password=""
    ), "login with empty password should be denied with 'without-nullok' enabled!"

    client.authselect.disable_feature(["without-nullok"])

    system_auth = client.fs.read("/etc/pam.d/system-auth")
    assert "nullok" in system_auth, "'pam_unix.so' should have 'nullok' after disabling 'without-nullok'!"
