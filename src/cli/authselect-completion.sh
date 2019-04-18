#
#    Authors:
#        Tomas Halman <thalman@redhat.com>
#
#    Copyright (C) 2019 Red Hat
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
#
# provides autocompletion for authselect command
#

_authselect_completions()
{
    local COMMANDS
    local command
    local possibleopts

    function is_valid_command() {
        local cmd

        for cmd in "${COMMANDS[@]}"; do
            if [[ "$cmd" = "$1" ]]; then
                return 0
            fi
        done
        return 1
    }

    function get_command() {
        local opt

        if [[ $COMP_CWORD -lt 2 ]] ; then
            return
        fi
        for opt in "${COMP_WORDS[@]:0:$COMP_CWORD}"; do
            if is_valid_command "$opt"; then
                echo "$opt"
                return
            fi
        done
    }

    function get_command_param() {
        local havecmd=0
        local len=${#COMP_WORDS[@]}-1

        if [[ "$command" = "" ]]; then
            return
        fi
        havecmd=0
        for (( i=0; i<$len; i++ )); do
            if [[ "$havecmd" = "1" ]] ; then
                if [[ "${COMP_WORDS[$i]}" =~ ^[-=] || "${COMP_WORDS[$i-1]}" = "=" ]] ; then
                    continue
                fi
                echo "${COMP_WORDS[$i]}"
                return
            fi
            if [[ "${COMP_WORDS[$i]}" = "$command" ]] ; then
                havecmd=1
            fi
        done
    }

    function get_profile() {
        case "$command" in
        select|show|requirements|test|list-features)
            get_command_param
            ;;
        enable-feature|disable-feature)
            authselect current 2>/dev/null | head -n1 | cut -d" " -f3
            ;;
        esac
    }

    function get_command_keywords() {
        local profile

        case "$command" in
            select|requirements|test)
                profile="$(get_profile)"
                if [[ "$profile" != "" ]] ; then
                    authselect list-features "$profile" 2>/dev/null
                fi
                ;;
        esac
    }

    function get_command_options() {
        if [[ "${COMP_WORDS[$COMP_CWORD]}" =~ ^- ]] ; then
            case "$command" in
                select)
                    echo "--force --quiet --nobackup --backup="
                    ;;
                apply-changes|disable-feature)
                    echo "--backup="
                    ;;
                enable-feature)
                    echo "--backup= --quiet"
                    ;;
                current|backup-list)
                    echo "--raw"
                    ;;
                create-profile)
                    echo "--vendor --base-on= --base-on-default" \
                         "--symlink-meta --symlink-nsswitch --symlink-pam" \
                         "--symlink-dconf --symlink="
                    ;;
                test)
                    echo "--all --nsswitch --system-auth --password-auth" \
                         "--smartcard-auth --fingerprint-auth --postlogin" \
                         "--dconf-db --dconf-lock"
                    ;;
            esac
        fi
    }

    function get_global_options() {
        if [[ "${COMP_WORDS[$COMP_CWORD]}" =~ ^- ]] ; then
            echo "--debug --trace --warn --help"
        fi
    }

    function get_option_params() {
        local opt

        if [[ $COMP_CWORD -gt 2 && "${COMP_WORDS[$COMP_CWORD-1]}" = "=" ]] ; then
            opt="${COMP_WORDS[$COMP_CWORD-2]}"
        else
            if [[ $COMP_CWORD -gt 1 ]] ; then
                opt="${COMP_WORDS[$COMP_CWORD-1]}"
            fi
        fi
        case "$opt" in
        --base-on)
            authselect list 2>/dev/null | cut -d" " -f2
            ;;
        --symlink)
            echo "dconf-db dconf-locks fingerprint-auth nsswitch.conf" \
                 "password-auth postlogin smartcard-auth system-auth" \
                 "README REQUIREMENTS"
            ;;
        esac

    }

    function get_command_params() {
        local i
        local profile

        if [[ "$command" = "" ]]; then
            return
        fi
        for (( i=$COMP_CWORD-1; i>1; i-- )); do
            opt="${COMP_WORDS[$i]}"
            if [[ "$opt" = "$command" ]] ; then
                break
            fi
            if [[ "$opt" =~ ^[-=] || "${COMP_WORDS[$i-1]}" = "=" ]] ; then
                continue
            fi
            return
        done
        case "$command" in
        select|show|requirements|test|list-features)
            authselect list 2>/dev/null | cut -d" " -f2
            ;;
        backup-remove|backup-restore)
            authselect backup-list 2>/dev/null | cut -d" " -f1
            ;;
        enable-feature|disable-feature)
            profile="$(get_profile)"
            if [[ "$profile" != "" ]] ; then
                authselect list-features "$profile" 2>/dev/null
            fi
            ;;
        esac
    }

    COMMANDS=(select apply-changes list list-features show requirements current
              check test enable-feature disable-feature create-profile
              backup-list backup-remove backup-restore)

    possibleopts="$(get_option_params)"
    if [[ "$possibleopts" != "" ]]; then
        if [[ "${COMP_WORDS[$COMP_CWORD]}" = "=" ]]; then
            COMPREPLY=($(compgen -W "$possibleopts"))
        else
            COMPREPLY=($(compgen -W "$possibleopts" -- "${COMP_WORDS[$COMP_CWORD]}"))
        fi
    else
        command="$(get_command)"
        if [[ "$command" = "" ]]; then
            possibleopts="$(get_global_options) ${COMMANDS[@]}"
        else
            possibleopts="$(get_global_options) $(get_command_params) $(get_command_keywords) $(get_command_options)"
        fi
        COMPREPLY=($(compgen -W "$possibleopts" -- "${COMP_WORDS[$COMP_CWORD]}"))
    fi
}

complete -F _authselect_completions authselect
