"""
Authselect Command Line Interface Tests

:requirement: Authselect replaced authconfig
"""

from __future__ import annotations

import pytest
from authselect_test_framework.profiles import Profile
from authselect_test_framework.roles.client import Client


@pytest.mark.importance("critical")
@pytest.mark.topology(Profile.Local)
def test_cli__select(client: Client):
    """
    :title: Sanity authselect select CLI output test
    :description:
        Verify 'select' exits without error and prints a profile-selected confirmation.
        An unknown profile name produces a non-zero exit code and stderr output.
    :setup:
    :steps:
        1. Run authselect select with a valid profile
        2. Run authselect select with an invalid profile
    :expectedresults:
        1. Return 0 with profile-selected confirmation in stdout and no stderr
        2. Return non-zero with stderr output
    :customerscenario: False
    """
    result = client.host.conn.run("authselect select minimal --force")
    assert result.rc == 0, "authselect select should return 0 for a valid profile!"
    assert "was selected" in result.stdout, "authselect select should confirm profile selection in stdout!"
    assert not result.stderr, "authselect select should produce no stderr on success!"

    result = client.host.conn.run("authselect select nonexistent-profile --force", raise_on_error=False)
    assert result.rc != 0, "authselect select should fail for an invalid profile!"
    assert result.stderr.strip(), "authselect select should produce stderr for an invalid profile!"


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.Local)
def test_cli__apply_changes(client: Client):
    """
    :title: Sanity authselect apply-changes CLI output test
    :description:
        Verify 'apply-changes' exits without error and prints a success confirmation.
    :setup:
        1. Select authselect profile
    :steps:
        1. Run authselect apply-changes
    :expectedresults:
        1. Return 0 with success confirmation in stdout and no stderr
    :customerscenario: False
    """
    client.authselect.select("minimal")

    result = client.host.conn.run("authselect apply-changes")
    assert result.rc == 0, "authselect apply-changes should return 0!"
    assert "successfully applied" in result.stdout, "authselect apply-changes should confirm success in stdout!"
    assert not result.stderr, "authselect apply-changes should produce no stderr on success!"


@pytest.mark.importance("critical")
@pytest.mark.topology(Profile.Local)
def test_cli__list(client: Client):
    """
    :title: Sanity authselect list CLI output test
    :description:
        Verify 'list' exits without error and outputs all built-in profiles.
    :setup:
    :steps:
        1. Run authselect list
    :expectedresults:
        1. Return 0 with all built-in profiles in stdout
    :customerscenario: False
    """
    result = client.host.conn.run("authselect list")
    assert result.rc == 0, "authselect list should return 0!"
    assert "sssd" in result.stdout, "authselect list should include 'sssd'!"
    assert "minimal" in result.stdout, "authselect list should include 'minimal'!"
    assert "winbind" in result.stdout, "authselect list should include 'winbind'!"
    assert not result.stderr, "authselect list should produce no stderr!"


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.Local)
def test_cli__list_features(client: Client):
    """
    :title: Sanity authselect list-features CLI output test
    :description:
        Verify 'list-features' exits without error and outputs the available
        features for the given profile.
    :setup:
    :steps:
        1. Run authselect list-features for the 'minimal' profile
    :expectedresults:
        1. Return 0 with known features in stdout
    :customerscenario: False
    """
    result = client.host.conn.run("authselect list-features minimal")
    assert result.rc == 0, "authselect list-features should return 0!"
    assert "with-mkhomedir" in result.stdout, "authselect list-features should include 'with-mkhomedir'!"
    assert "with-faillock" in result.stdout, "authselect list-features should include 'with-faillock'!"
    assert not result.stderr, "authselect list-features should produce no stderr!"


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.Local)
def test_cli__show(client: Client):
    """
    :title: Sanity authselect show CLI output test
    :description:
        Verify 'show' exits without error and outputs the profile documentation.
    :setup:
    :steps:
        1. Run authselect show for the 'minimal' profile
    :expectedresults:
        1. Return 0 with the profile description in stdout
    :customerscenario: False
    """
    result = client.host.conn.run("authselect show minimal")
    assert result.rc == 0, "authselect show should return 0!"
    assert "Local users only" in result.stdout, "authselect show should include 'Local users only'!"
    assert not result.stderr, "authselect show should produce no stderr!"


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.Local)
def test_cli__requirements(client: Client):
    """
    :title: Sanity authselect requirements CLI output test
    :description:
        Verify 'requirements' exits without error and outputs the profile
        requirements.
    :setup:
    :steps:
        1. Run authselect requirements for the 'minimal' profile
    :expectedresults:
        1. Return 0 with requirements in stdout
    :customerscenario: False
    """
    result = client.host.conn.run("authselect requirements minimal")
    assert result.rc == 0, "authselect requirements should return 0!"
    assert (
        "No requirements are specified." in result.stdout
    ), "authselect requirements should include 'No requirements are specified.' for 'minimal' profile!"
    assert not result.stderr, "authselect requirements should produce no stderr!"


@pytest.mark.importance("critical")
@pytest.mark.topology(Profile.Local)
def test_cli__current(client: Client):
    """
    :title: Sanity authselect current CLI output test
    :description:
        Verify 'current' exits without error and outputs the active profile name
        and its enabled features. The '--raw' flag outputs a single
        machine-readable line.
    :setup:
        1. Select authselect profile with 'with-mkhomedir' feature
    :steps:
        1. Run authselect current
        2. Run authselect current --raw
    :expectedresults:
        1. Return 0 with 'minimal' profile ID and 'with-mkhomedir' feature in stdout
        2. Return 0 with 'minimal with-mkhomedir' as the only line
    :customerscenario: False
    """
    client.authselect.select("minimal", ["with-mkhomedir"])

    result = client.host.conn.run("authselect current")
    assert result.rc == 0, "authselect current should return 0!"
    assert "Profile ID: minimal" in result.stdout, "authselect current should show 'Profile ID: minimal'!"
    assert "with-mkhomedir" in result.stdout, "authselect current should show 'with-mkhomedir'!"
    assert not result.stderr, "authselect current should produce no stderr!"

    result = client.host.conn.run("authselect current --raw")
    assert result.rc == 0, "authselect current --raw should return 0!"
    assert (
        result.stdout.strip() == "minimal with-mkhomedir"
    ), "authselect current --raw should output 'minimal with-mkhomedir'!"
    assert not result.stderr, "authselect current --raw should produce no stderr!"


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.Local)
def test_cli__check(client: Client):
    """
    :title: Sanity authselect check CLI output test
    :description:
        Verify 'check' exits without error and outputs a configuration validity
        message.
    :setup:
        1. Select authselect profile
    :steps:
        1. Run authselect check
    :expectedresults:
        1. Return 0 with a valid configuration message in stdout
    :customerscenario: False
    """
    client.authselect.select("minimal")

    result = client.host.conn.run("authselect check")
    assert result.rc == 0, "authselect check should return 0 for valid configuration!"
    assert "valid" in result.stdout, "authselect check should report configuration is 'valid'!"
    assert not result.stderr, "authselect check should produce no stderr!"


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.Local)
def test_cli__test(client: Client):
    """
    :title: Sanity authselect test CLI output test
    :description:
        Verify 'test' exits without error and outputs the file paths that would
        be generated.
    :setup:
    :steps:
        1. Run authselect test for the 'minimal' profile
    :expectedresults:
        1. Return 0 with expected file paths in stdout
    :customerscenario: False
    """
    result = client.host.conn.run("authselect test minimal")
    assert result.rc == 0, "authselect test should return 0!"
    assert "/etc/nsswitch.conf" in result.stdout, "authselect test should include '/etc/nsswitch.conf'!"
    assert not result.stderr, "authselect test should produce no stderr!"


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.Local)
def test_cli__enable_feature(client: Client):
    """
    :title: Sanity authselect enable-feature CLI output test
    :description:
        Verify 'enable-feature' exits without error. Advisory notes may appear on stdout.
        The feature appears in the 'current --raw' output.
    :setup:
        1. Select authselect profile without features
    :steps:
        1. Run authselect enable-feature with-mkhomedir
        2. Verify the feature is reflected in current profile
    :expectedresults:
        1. Return 0 with no stderr
        2. 'with-mkhomedir' appears in authselect current --raw output
    :customerscenario: False
    """
    client.authselect.select("minimal")

    result = client.host.conn.run("authselect enable-feature with-mkhomedir")
    assert result.rc == 0, "authselect enable-feature should return 0!"
    assert not result.stderr, "authselect enable-feature should produce no stderr on success!"

    result = client.host.conn.run("authselect current --raw")
    assert (
        "with-mkhomedir" in result.stdout
    ), "authselect current --raw should include 'with-mkhomedir' after enable-feature!"


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.Local)
def test_cli__disable_feature(client: Client):
    """
    :title: Sanity authselect disable-feature CLI output test
    :description:
        Verify 'disable-feature' exits without error and produces no output.
        The feature is absent from the 'current --raw' output.
    :setup:
        1. Select authselect profile with 'with-mkhomedir' feature
    :steps:
        1. Run authselect disable-feature with-mkhomedir
        2. Verify the feature is no longer reflected in current profile
    :expectedresults:
        1. Return 0 with no stdout or stderr
        2. 'with-mkhomedir' does not appear in authselect current --raw output
    :customerscenario: False
    """
    client.authselect.select("minimal", ["with-mkhomedir"])

    result = client.host.conn.run("authselect disable-feature with-mkhomedir")
    assert result.rc == 0, "authselect disable-feature should return 0!"
    assert not result.stdout, "authselect disable-feature should produce no stdout on success!"
    assert not result.stderr, "authselect disable-feature should produce no stderr on success!"

    result = client.host.conn.run("authselect current --raw")
    assert (
        "with-mkhomedir" not in result.stdout
    ), "authselect current --raw should not include 'with-mkhomedir' after disable-feature!"


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.Local)
def test_cli__create_profile(client: Client):
    """
    :title: Sanity authselect create-profile CLI output test
    :description:
        Verify 'create-profile' exits without error and the new profile appears
        in the 'list' output.
    :setup:
    :steps:
        1. Run authselect create-profile based on the 'minimal' profile
        2. Verify the new profile appears in authselect list
    :expectedresults:
        1. Return 0 with no stderr
        2. 'cli-test-profile' appears in authselect list output
    :customerscenario: False
    """
    result = client.host.conn.run("authselect create-profile cli-test-profile --base-on minimal")
    assert result.rc == 0, "authselect create-profile should return 0!"
    assert not result.stderr, "authselect create-profile should produce no stderr!"

    result = client.host.conn.run("authselect list")
    assert (
        "cli-test-profile" in result.stdout
    ), "authselect list should include 'cli-test-profile' after create-profile!"

    client.host.conn.run("rm -rf /etc/authselect/custom/cli-test-profile")


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.Local)
def test_cli__backup_list(client: Client):
    """
    :title: Sanity authselect backup-list CLI output test
    :description:
        Verify 'backup-list' exits without error.
    :setup:
    :steps:
        1. Run authselect backup-list
    :expectedresults:
        1. Return 0 with no stderr
    :customerscenario: False
    """
    result = client.host.conn.run("authselect backup-list")
    assert result.rc == 0, "authselect backup-list should return 0!"
    assert not result.stderr, "authselect backup-list should produce no stderr!"


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.Local)
def test_cli__backup_restore(client: Client):
    """
    :title: Sanity authselect backup-restore CLI output test
    :description:
        Verify 'backup-restore' exits without error and produces no output.
        The restored profile appears in the 'current --raw' output.
    :setup:
        1. Select authselect profile creating a named backup
    :steps:
        1. Run authselect backup-restore
        2. Verify the restored profile is current
    :expectedresults:
        1. Return 0 with no stdout or stderr
        2. The restored 'minimal' profile is shown by authselect current
    :customerscenario: False
    """
    client.host.conn.run("authselect select minimal --backup=cli-test-backup --force")

    result = client.host.conn.run("authselect backup-restore cli-test-backup")
    assert result.rc == 0, "authselect backup-restore should return 0!"
    assert not result.stdout, "authselect backup-restore should produce no stdout on success!"
    assert not result.stderr, "authselect backup-restore should produce no stderr on success!"

    result = client.host.conn.run("authselect current --raw")
    assert "minimal" in result.stdout, "'minimal' profile should be current after backup-restore!"

    client.host.conn.run("authselect backup-remove cli-test-backup")


@pytest.mark.importance("high")
@pytest.mark.topology(Profile.Local)
def test_cli__backup_remove(client: Client):
    """
    :title: Sanity authselect backup-remove CLI output test
    :description:
        Verify 'backup-remove' exits without error and produces no output.
        The removed backup is absent from the 'backup-list' output.
    :setup:
        1. Select authselect profile creating a named backup
    :steps:
        1. Run authselect backup-remove
        2. Verify the backup is no longer listed
    :expectedresults:
        1. Return 0 with no stdout or stderr
        2. 'cli-test-backup' does not appear in authselect backup-list
    :customerscenario: False
    """
    client.host.conn.run("authselect select minimal --backup=cli-test-backup --force")

    result = client.host.conn.run("authselect backup-remove cli-test-backup")
    assert result.rc == 0, "authselect backup-remove should return 0!"
    assert not result.stdout, "authselect backup-remove should produce no stdout on success!"
    assert not result.stderr, "authselect backup-remove should produce no stderr on success!"

    result = client.host.conn.run("authselect backup-list")
    assert (
        "cli-test-backup" not in result.stdout
    ), "authselect backup-list should not include 'cli-test-backup' after backup-remove!"
