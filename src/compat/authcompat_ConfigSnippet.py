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

import re


class ConfigSnippet:
    TEST = False

    AllKeysRE = re.compile('\${\??(?P<key>[\w-]*)}')
    DummyKeysRE = re.compile('\${\?[\w-]*}')

    def __init__(self, template, destination):
        with open(template, "r") as f:
            self.template = f.read()

        self.destination = destination

    def generate(self, values):
        # First remove lines containing key that is not set
        lines = self.template.split('\n')
        remove = []

        for idx, line in enumerate(lines):
            for match in self.AllKeysRE.finditer(line):
                key = match.group("key")
                if key not in values or values[key] is None:
                    remove.append(idx)
                    break

        for idx in sorted(remove, reverse=True):
            del lines[idx]

        # Build output string
        output = '\n'.join(lines)

        # Remove all dummy keys ${?key}
        output = self.DummyKeysRE.sub("", output)

        # Replace values
        for key, value in values.items():
            if value is None:
                continue

            if type(value) is bool:
                value = "true" if value else "false"

            output = output.replace("${%s}" % key, value)

        return output

    def write(self, values, to_stdout=False):
        output = self.generate(values)

        if self.TEST:
            print("========== BEGIN Content of [%s] ==========" % self.destination)
            print(output)
            print("========== END   Content of [%s] ==========\n" % self.destination)
            return

        with open(self.destination, "w") as f:
            f.write(output)
