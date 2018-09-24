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

import os
import re

class EnvironmentFile:
    TEST = False

    def __init__(self, filename,
                 delimiter='=', delimiter_re=None,
                 quotes=True):
        self.filename = filename
        self.delimiter = delimiter
        self.quotes = quotes
        self.environment = []

        delimiter_re = delimiter_re if delimiter_re is not None else delimiter
        self.pattern = re.compile('^(\s*)(\S*)([^\n\S]*)(' +
                                  delimiter_re +
                                  ')([^\n\S]*)(.*)$',
                                  re.MULTILINE)

        self.read()

    def read(self):
        try:
            with open(self.filename, "r") as f:
                lines = f.readlines()
        except FileNotFoundError:
            return

        for line in lines:
            parsed = self.Line.Parse(line, self.pattern,
                                     self.delimiter, self.quotes)
            self.environment.append(parsed)

    def write(self):
        output = ""
        for line in self.environment:
            output = output + line.getLine()

        if self.TEST:
            print("========== BEGIN Content of [%s] ==========" % self.filename)
            print(output)
            print("========== END   Content of [%s] ==========\n" % self.filename)
            return

        dirname = os.path.dirname(self.filename)
        if not os.path.exists(dirname):
            try:
                os.makedirs(dirname)
            except OSError as exception:
                if exception.errno == errno.EEXIST and os.path.isdir(dirname):
                    pass
                else:
                    raise

        with open(self.filename, "w") as f:
            f.write(output)

    def get(self, name, default=None):
        value = None
        for line in self.environment:
            if line.isVariable() and line.name == name:
                value = line.value

        if value is None:
            return default

        if value.lower() in [None, "no", "false", "f", "n"]:
            return False
        elif value.lower() in ["yes", "true", "t", "y"]:
            return True

        return value

    def getall(self):
        lines = []
        for line in self.environment:
            if line.isVariable():
                lines.append(line)

        return lines

    def set(self, name, value):
        if type(value) is bool:
            value = "yes" if value else "no"

        for line in self.environment:
            if line.isVariable() and line.name == name:
                line.set(name, value)
                return

        line = self.Line(self.delimiter, self.quotes)
        line.set(name, value)
        self.environment.append(line)

    class Line:
        def __init__(self, delimiter, quotes,
                     name=None, value=None, original=None, fmt=None):
            self.delimiter = delimiter
            self.quotes = quotes
            self.name = name
            self.value = value
            self.original = original
            self.fmt = fmt

        def isVariable(self):
            return self.fmt is not None

        def isOriginal(self):
            return self.original is not None

        def set(self, name, value):
            self.name = name
            self.value = value
            if self.fmt is None:
                self.fmt = "${name}%s${value}\n" % self.delimiter

        def getLine(self):
            if self.isOriginal():
                return self.original

            value = self.value if not self.value is None else ""
            replacement = {
                'name': self.name,
                'value': self.Escape(value, self.quotes)
            }

            line = self.fmt
            for key, value in replacement.items():
                line = line.replace("${" + key + "}", str(value))

            return line

        @staticmethod
        def Parse(line, pattern, delimiter, quotes):
            match = pattern.match(line)
            if line.startswith('#') or not line.strip() or not match:
                return EnvironmentFile.Line(delimiter, quotes, original=line)

            name = match.group(2)
            value = EnvironmentFile.Line.Unescape(match.group(6), quotes)
            fmt = "%s${name}%s%s%s${value}\n" % (match.group(1),
                                                 match.group(3),
                                                 match.group(4),
                                                 match.group(5))

            return EnvironmentFile.Line(delimiter, quotes, name=name,
                                        value=value, fmt=fmt)

        @staticmethod
        def Escape(value, quotes):
            if value is None:
                return ""

            value = str(value)
            value = value.replace("\\", "\\\\")
            value = value.replace("\"", "\\\"")
            value = value.replace("'", "\\\'")
            value = value.replace("$", "\\\$")
            value = value.replace("~", "\\\~")
            value = value.replace("`", "\\\`")

            if quotes:
                if value.find(" ") > 0 or value.find("\t") > 0:
                    value = "\"" + value + "\""

            return value

        @staticmethod
        def Unescape(value, quotes):
            if not value:
                return value

            value = str(value)

            length = len(value)
            if quotes:
                if (value[0] == "\"" or value[0] == "'") and value[0] == value[length - 1]:
                    value = value[1:length - 1]

            i = 0
            while True:
                i = value.find("\\", i)
                if i < 0:
                    break
                if i + 1 >= len(value):
                    value = value[0:i]
                    break

                value = value[0:i] + value[i + 1:]
                i += 1

            return value
