# Configuration file for multihost tests.

from __future__ import annotations

from authselect_test_framework.config import AuthselectMultihostConfig
from pytest_mh import MultihostPlugin

pytest_plugins = (
    "pytest_importance",
    "pytest_mh",
    "pytest_ticket",
    "pytest_tier",
    "authselect_test_framework.fixtures",
    "authselect_test_framework.markers",
)


def pytest_plugin_registered(plugin) -> None:
    if isinstance(plugin, MultihostPlugin):
        plugin.config_class = AuthselectMultihostConfig
