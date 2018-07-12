# -*- coding: utf-8 -*-
#
#    Authors:
#        Pavel Březina <pbrezina@redhat.com>
#
#    Copyright (C) 2018 Red Hat
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import argparse
import gettext

_ = gettext.gettext


class Option:
    def __init__(self, name, metavar, help, feature, supported):
        self.name = name
        self.metavar = metavar
        self.help = help
        self.feature = feature
        self.supported = supported
        self.value = None
        self.from_sysconfig = False

    def set(self, new_value):
        self.value = new_value

    def set_from_sysconfig(self, new_value):
        self.set(new_value)
        self.from_sysconfig = True

    def isset(self):
        return self.value is not None

    @staticmethod
    def Valued(name, metavar, help):
        return Option(name, metavar, help, feature=False, supported=True)

    @staticmethod
    def Switch(name, help):
        return Option(name, None, help, feature=False, supported=True)

    @staticmethod
    def Feature(name, help):
        return Option(name, None, help, feature=True, supported=True)

    @staticmethod
    def UnsupportedValued(name, metavar):
        return Option(name, metavar, None, feature=False, supported=False)

    @staticmethod
    def UnsupportedFeature(name):
        return Option(name, None, None, feature=True, supported=False)

    @staticmethod
    def UnsupportedSwitch(name):
        return Option(name, None, None, feature=False, supported=False)


class Options:
    List = [
        # These options are still supported in authconfig compatibility
        # layers. The tool will do its best to translate them to authselect
        # call and where needed, it will generate a configuration file.
        #
        # However, they will just make sure that an authentication against
        # expected service is working. They may not result in the exact same
        # configuration as authconfig would generate.
        Option.Valued ("nisdomain",       _("<domain>"), _("default NIS domain")),
        Option.Feature("ldap",            _("LDAP for user information by default")),
        Option.Feature("ldapauth",        _("LDAP for authentication by default")),
        Option.Valued ("ldapserver",      _("<server>"), _("default LDAP server hostname or URI")),
        Option.Valued ("ldapbasedn",      _("<dn>"), _("default LDAP base DN")),
        Option.Feature("ldaptls",         _("use of TLS with LDAP (RFC-2830)")),
        Option.Feature("ldapstarttls",    _("use of TLS for identity lookups with LDAP (RFC-2830)")),
        Option.Feature("rfc2307bis",      _("use of RFC-2307bis schema for LDAP user information lookups")),
        Option.Feature("smartcard",       _("authentication with smart card by default")),
        Option.Valued ("smartcardaction", _("<0=Lock|1=Ignore>"), _("action to be taken on smart card removal")),
        Option.Feature("fingerprint",     _("authentication with fingerprint readers by default")),
        Option.Feature("ecryptfs",        _("automatic per-user ecryptfs")),
        Option.Feature("krb5",            _("Kerberos authentication by default")),
        Option.Valued ("krb5kdc",         _("<server>"), _("default Kerberos KDC")),
        Option.Valued ("krb5adminserver", _("<server>"), _("default Kerberos admin server")),
        Option.Valued ("krb5realm",       _("<realm>"), _("default Kerberos realm")),
        Option.Feature("krb5kdcdns",      _("use of DNS to find Kerberos KDCs")),
        Option.Feature("krb5realmdns",    _("use of DNS to find Kerberos realms")),
        Option.Feature("winbind",         _("winbind for user information by default")),
        Option.Feature("winbindauth",     _("winbind for authentication by default")),
        Option.Valued ("winbindjoin",     _("<Administrator>"), _("join the winbind domain or ads realm now as this administrator")),
        Option.Feature("winbindkrb5",     _("Kerberos 5 for authenticate with winbind")),
        Option.Valued ("smbworkgroup",    _("<workgroup>"), _("workgroup authentication servers are in")),
        Option.Feature("sssd",            _("SSSD for user information by default with manually managed configuration")),
        Option.Feature("sssdauth",        _("SSSD for authentication by default with manually managed configuration")),
        Option.Feature("cachecreds",      _("caching of user credentials in SSSD by default")),
        Option.Feature("pamaccess",       _("check of access.conf during account authorization")),
        Option.Feature("mkhomedir",       _("creation of home directories for users on their first login")),
        Option.Feature("faillock",        _("account locking in case of too many consecutive authentication failures")),
        Option.Valued ("passminlen",      _("<number>"), _("minimum length of a password")),
        Option.Valued ("passminclass",    _("<number>"), _("minimum number of character classes in a password")),
        Option.Valued ("passmaxrepeat",   _("<number>"), _("maximum number of same consecutive characters in a password")),
        Option.Valued ("passmaxclassrepeat", _("<number>"), _("maximum number of consecutive characters of same class in a password")),
        Option.Feature("reqlower",        _("require at least one lowercase character in a password")),
        Option.Feature("requpper",        _("require at least one uppercase character in a password")),
        Option.Feature("reqdigit",        _("require at least one digit in a password")),
        Option.Feature("reqother",        _("require at least one other character in a password")),

        # Program options
        Option.Switch ("nostart",         _("do not start/stop services")),
        Option.Switch ("updateall",       _("update all configuration files")),
        Option.Switch ("update",          _("the same as --updateall")),
        Option.Switch ("kickstart",       _("the same as --updateall")),

        # Hidden compat tool option, useful for testing. No changes to the
        # system will be done, they will be printed.
        Option.Switch ("test-call",       argparse.SUPPRESS),

        # Unsupported program options but we have to react somehow when set
        Option.UnsupportedSwitch("test"),
        Option.UnsupportedSwitch("probe"),
        Option.UnsupportedValued("savebackup", _("<name>")),
        Option.UnsupportedValued("restorebackup", _("<name>")),
        Option.UnsupportedSwitch("restorelastbackup"),

        # These options are no longer supported in authconfig compatibility
        # layers and will produce warning when used. They will not affect
        # the system.
        Option.UnsupportedFeature("cache"),
        Option.UnsupportedFeature("shadow"),
        Option.UnsupportedSwitch ("useshadow"),
        Option.UnsupportedFeature("md5"),
        Option.UnsupportedSwitch ("usemd5"),
        Option.UnsupportedValued ("passalgo", _("<descrypt|bigcrypt|md5|sha256|sha512>")),
        Option.UnsupportedFeature("nis"),
        Option.UnsupportedValued ("nisserver", _("<server>")),
        Option.UnsupportedValued ("ldaploadcacert", _("<URL>")),
        Option.UnsupportedFeature("requiresmartcard"),
        Option.UnsupportedValued ("smartcardmodule", _("<module>")),
        Option.UnsupportedValued ("smbsecurity", _("<user|server|domain|ads>")),
        Option.UnsupportedValued ("smbrealm", _("<realm>")),
        Option.UnsupportedValued ("smbservers", _("<servers>")),
        Option.UnsupportedValued ("smbidmaprange", _("<lowest-highest>")),
        Option.UnsupportedValued ("smbidmapuid", _("<lowest-highest>")),
        Option.UnsupportedValued ("smbidmapgid", _("<lowest-highest>")),
        Option.UnsupportedValued ("winbindseparator", _("<\>")),
        Option.UnsupportedValued ("winbindtemplatehomedir", _("</home/%D/%U>")),
        Option.UnsupportedValued ("winbindtemplateshell", _("</bin/false>")),
        Option.UnsupportedFeature("winbindusedefaultdomain"),
        Option.UnsupportedFeature("winbindoffline"),
        Option.UnsupportedFeature("preferdns"),
        Option.UnsupportedFeature("forcelegacy"),
        Option.UnsupportedFeature("locauthorize"),
        Option.UnsupportedFeature("sysnetauth"),
        Option.UnsupportedValued ("faillockargs", _("<options>")),
    ]

    Map = {
        # These options were use with autodetection of pam_cracklib
        # and pam_passwdqc. However, authselect supports only pam_pwquality.
        # "USEPWQUALITY"     : "",
        # "USEPASSWDQC"      : "",
        "USEFAILLOCK"      : "faillock",
        "FAILLOCKARGS"     : "faillockargs",
        "USELDAP"          : "ldap",
        "USENIS"           : "nis",
        "USEECRYPTFS"      : "ecryptfs",
        "USEWINBIND"       : "winbind",
        "WINBINDKRB5"      : "winbindkrb5",
        "USESSSD"          : "sssd",
        "USEKERBEROS"      : "krb5",
        "USELDAPAUTH"      : "ldapauth",
        "USESMARTCARD"     : "smartcard",
        "FORCESMARTCARD"   : "requiresmartcard",
        "USEFPRINTD"       : "fingerprint",
        "PASSWDALGORITHM"  : "passalgo",
        "USEMD5"           : "md5",
        "USESHADOW"        : "shadow",
        "USEWINBINDAUTH"   : "winbindauth",
        "USESSSDAUTH"      : "sssdauth",
        "USELOCAUTHORIZE"  : "locauthorize",
        "USEPAMACCESS"     : "pamaccess",
        "USEMKHOMEDIR"     : "mkhomedir",
        "USESYSNETAUTH"    : "sysnetauth",
        "FORCELEGACY"      : "forcelegacy",
        "CACHECREDENTIALS" : "cachecreds",
    }

    def __init__(self):
        self.options = {}

        for option in self.List:
            self.options[option.name] = option

    def parse(self):
        parser = argparse.ArgumentParser(description='Authconfig Compatibility Tool.')

        parsers = {
            'supported'   : parser.add_argument_group(_('These options have a compatibility layer')),
            'unsupported' : parser.add_argument_group(_('These options are no longer supported and have no effect'))
        }

        for option in self.List:
            group = 'supported' if option.supported else 'unsupported'
            self.add_option(parsers[group], option)

        cmdline = parser.parse_args()

        for name, option in self.options.items():
            value = getattr(cmdline, name)
            option.set(value)

        # usemd5 and useshadow are equivalent to enablemd5 and enableshadow
        if not self.isset('md5') and self.isset('usemd5'):
            self.set('md5', self.get('usemd5'))

        if not self.isset('shadow') and self.isset('useshadow'):
            self.set('shadow', self.get('useshadow'))

        # ldapstarttls is equivalent to ldaptls
        if self.isset('ldapstarttls') and not self.isset('ldaptls'):
            self.set('ldaptls', self.get('ldapstarttls'))

    def applysysconfig(self, sysconfig):
        for name, option in self.Map.items():
            if not self.isset(option):
                self.options[option].set_from_sysconfig(sysconfig.get(name))

    def updatesysconfig(self, sysconfig):
        for name, option in self.Map.items():
            if self.isset(option):
                sysconfig.set(name, self.get(option))

    def get(self, name):
        return self.options[name].value

    def set(self, name, value):
        self.options[name].set(value)

    def isset(self, name):
        return self.options[name].isset()

    def getBool(self, name):
        value = self.get(name)
        if value is None or not value:
            return False
        return True

    def getTrueOrNone(self, name):
        value = self.get(name)
        if value is None or not value:
            return None
        return True

    def getSetButUnsupported(self):
        options = []
        for option in Options.List:
            if option.supported:
                continue

            if not option.isset():
                continue

            if option.from_sysconfig:
                continue

            name = option.name
            if option.feature:
                name = "enable" + name if option.value else "disable" + name

            options.append(name)

        return options

    def add_option(self, parser, option):
        if option.metavar is not None:
            self.add_valued(parser, option)
        elif option.feature:
            self.add_feature(parser, option)
        else:
            self.add_switch(parser, option)

    def add_valued(self, parser, option):
        parser.add_argument("--"+option.name,
                            action='store',
                            help=option.help,
                            dest=option.name,
                            metavar=option.metavar)

    def add_switch(self, parser, option):
        parser.add_argument("--"+option.name,
                            action='store_const',
                            const=True,
                            help=option.help,
                            dest=option.name)

    def add_feature(self, parser, option):
        help_enable = None
        help_disable = None

        if option.help is not None:
            help_enable = _("enable") + " " + option.help
            help_disable = _("disable") + " " + option.help

        parser.add_argument("--enable" + option.name,
                            action='store_const',
                            const=True,
                            help=help_enable,
                            dest=option.name)

        parser.add_argument("--disable" + option.name,
                            action='store_const',
                            const=False,
                            help=help_disable,
                            dest=option.name)
