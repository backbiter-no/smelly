#!/bin/bash

if [[ "$-" != *i* ]] ; then builtin return; fi  # check in interactive mode
if [[ -z "$smelly_SHELL_INTEGRATION" ]]; then builtin return; fi

# Load the normal bash startup files
if [[ -n "$smelly_BASH_INJECT" ]]; then
    builtin declare smelly_bash_inject="$smelly_BASH_INJECT"
    builtin declare ksi_val="$smelly_SHELL_INTEGRATION"
    builtin unset smelly_SHELL_INTEGRATION  # ensure manual sourcing of this file in bashrc does not have any effect
    builtin unset smelly_BASH_INJECT ENV
    if [[ -z "$HOME" ]]; then HOME=~; fi
    if [[ -z "$smelly_BASH_ETC_LOCATION" ]]; then smelly_BASH_ETC_LOCATION="/etc"; fi

    _ksi_sourceable() {
        [[ -f "$1" && -r "$1" ]] && builtin return 0; builtin return 1;
    }

    if [[ "$smelly_bash_inject" == *"posix"* ]]; then
        _ksi_sourceable "$smelly_BASH_POSIX_ENV" && {
            builtin source "$smelly_BASH_POSIX_ENV"
            builtin export ENV="$smelly_BASH_POSIX_ENV"
        }
    else
        builtin set +o posix
        builtin shopt -u inherit_errexit 2>/dev/null  # resetting posix does not clear this
        if [[ -n "$smelly_BASH_UNEXPORT_HISTFILE" ]]; then
            builtin export -n HISTFILE
            builtin unset smelly_BASH_UNEXPORT_HISTFILE
        fi

        # See run_startup_files() in shell.c in the Bash source code
        if builtin shopt -q login_shell; then
            if [[ "$smelly_bash_inject" != *"no-profile"* ]]; then
                _ksi_sourceable "$smelly_BASH_ETC_LOCATION/profile" && builtin source "$smelly_BASH_ETC_LOCATION/profile"
                for _ksi_i in "$HOME/.bash_profile" "$HOME/.bash_login" "$HOME/.profile"; do
                    _ksi_sourceable "$_ksi_i" && { builtin source "$_ksi_i"; break; }
                done
            fi
        else
            if [[ "$smelly_bash_inject" != *"no-rc"* ]]; then
                # Linux distros build bash with -DSYS_BASHRC. Unfortunately, there is
                # no way to to probe bash for it and different distros use different files
                # Arch, Debian, Ubuntu use /etc/bash.bashrc
                # Fedora uses /etc/bashrc sourced from ~/.bashrc instead of SYS_BASHRC
                # Void Linux uses /etc/bash/bashrc
                for _ksi_i in "$smelly_BASH_ETC_LOCATION/bash.bashrc" "$smelly_BASH_ETC_LOCATION/bash/bashrc" ; do
                    _ksi_sourceable "$_ksi_i" && { builtin source "$_ksi_i"; break; }
                done
                if [[ -z "$smelly_BASH_RCFILE" ]]; then smelly_BASH_RCFILE="$HOME/.bashrc"; fi
                _ksi_sourceable "$smelly_BASH_RCFILE" && builtin source "$smelly_BASH_RCFILE"
            fi
        fi
    fi
    builtin unset smelly_BASH_RCFILE smelly_BASH_POSIX_ENV smelly_BASH_ETC_LOCATION
    builtin unset -f _ksi_sourceable
    builtin export smelly_SHELL_INTEGRATION="$ksi_val"
    builtin unset _ksi_i ksi_val smelly_bash_inject
fi


if [ "${BASH_VERSINFO:-0}" -lt 4 ]; then
    builtin unset smelly_SHELL_INTEGRATION
    builtin printf "%s\n" "Bash version ${BASH_VERSION} too old, smelly shell integration disabled" > /dev/stderr
    builtin return
fi

if [[ "${_ksi_prompt[sourced]}" == "y" ]]; then
    # we have already run
    builtin unset smelly_SHELL_INTEGRATION
    builtin return
fi

# this is defined outside _ksi_main to make it global without using declare -g
# which is not available on older bash
builtin declare -A _ksi_prompt
_ksi_prompt=(
    [cursor]='y' [title]='y' [mark]='y' [complete]='y' [cwd]='y' [ps0]='' [ps0_suffix]='' [ps1]='' [ps1_suffix]='' [ps2]=''
    [hostname_prefix]='' [sourced]='y' [last_reported_cwd]=''
)

_ksi_main() {
    builtin local ifs="$IFS"
    IFS=" "
    for i in ${smelly_SHELL_INTEGRATION[@]}; do
        case "$i" in
            "no-cursor") _ksi_prompt[cursor]='n';;
            "no-title") _ksi_prompt[title]='n';;
            "no-prompt-mark") _ksi_prompt[mark]='n';;
            "no-complete") _ksi_prompt[complete]='n';;
            "no-cwd") _ksi_prompt[cwd]='n';;
        esac
    done
    IFS="$ifs"

    builtin unset smelly_SHELL_INTEGRATION

    _ksi_debug_print() {
        # print a line to STDERR of parent smelly process
        builtin local b
        b=$(builtin command base64 <<< "${@}")
        builtin printf "\eP@smelly-print|%s\e\\" "${b//[[:space:]]}}"
    }

    _ksi_set_mark() {
        _ksi_prompt["${1}_mark"]="\[\e]133;k;${1}_smelly\a\]"
    }

    _ksi_set_mark start
    _ksi_set_mark end
    _ksi_set_mark start_secondary
    _ksi_set_mark end_secondary
    _ksi_set_mark start_suffix
    _ksi_set_mark end_suffix
    builtin unset -f _ksi_set_mark
    _ksi_prompt[secondary_prompt]="\n${_ksi_prompt[start_secondary_mark]}\[\e]133;A;k=s\a\]${_ksi_prompt[end_secondary_mark]}"

    _ksi_prompt_command() {
        # we first remove any previously added smelly code from the prompt variables and then add
        # it back, to ensure we have only a single instance
        if [[ -n "${_ksi_prompt[ps0]}" ]]; then
            PS0=${PS0//\\\[\\e\]133;k;start_smelly\\a\\\]*end_smelly\\a\\\]}
            PS0="${_ksi_prompt[ps0]}$PS0"
        fi
        if [[ -n "${_ksi_prompt[ps0_suffix]}" ]]; then
            PS0=${PS0//\\\[\\e\]133;k;start_suffix_smelly\\a\\\]*end_suffix_smelly\\a\\\]}
            PS0="${PS0}${_ksi_prompt[ps0_suffix]}"
        fi
        # restore PS1 to its pristine state without our additions
        if [[ -n "${_ksi_prompt[ps1]}" ]]; then
            PS1=${PS1//\\\[\\e\]133;k;start_smelly\\a\\\]*end_smelly\\a\\\]}
            PS1=${PS1//\\\[\\e\]133;k;start_secondary_smelly\\a\\\]*end_secondary_smelly\\a\\\]}
        fi
        if [[ -n "${_ksi_prompt[ps1_suffix]}" ]]; then
            PS1=${PS1//\\\[\\e\]133;k;start_suffix_smelly\\a\\\]*end_suffix_smelly\\a\\\]}
        fi
        if [[ -n "${_ksi_prompt[ps1]}" ]]; then
            if [[ "${_ksi_prompt[mark]}" == "y" && ( "${PS1}" == *"\n"* || "${PS1}" == *$'\n'* ) ]]; then
                builtin local oldval
                oldval=$(builtin shopt -p extglob)
                builtin shopt -s extglob
                # bash does not redraw the leading lines in a multiline prompt so
                # mark the last line as a secondary prompt. Otherwise on resize the
                # lines before the last line will be erased by smelly.
                # the first part removes everything from the last \n onwards
                # the second part appends a newline with the secondary marking
                # the third part appends everything after the last newline
                PS1=${PS1%@('\n'|$'\n')*}${_ksi_prompt[secondary_prompt]}${PS1##*@('\n'|$'\n')}
                builtin eval "$oldval"
            fi
            PS1="${_ksi_prompt[ps1]}$PS1"
        fi
        if [[ -n "${_ksi_prompt[ps1_suffix]}" ]]; then
            PS1="${PS1}${_ksi_prompt[ps1_suffix]}"
        fi
        if [[ -n "${_ksi_prompt[ps2]}" ]]; then
            PS2=${PS2//\\\[\\e\]133;k;start_smelly\\a\\\]*end_smelly\\a\\\]}
            PS2="${_ksi_prompt[ps2]}$PS2"
        fi

        if [[ "${_ksi_prompt[cwd]}" == "y" ]]; then
            # unfortunately bash provides no hooks to detect cwd changes
            # in particular this means cwd reporting will not happen for a
            # command like cd /test && cat. PS0 is evaluated before cd is run.
            if [[ "${_ksi_prompt[last_reported_cwd]}" != "$PWD" ]]; then
                _ksi_prompt[last_reported_cwd]="$PWD"
                builtin printf "\e]7;smelly-shell-cwd://%s%s\a" "$HOSTNAME" "$PWD"
            fi
        fi
    }

    if [[ "${_ksi_prompt[cursor]}" == "y" ]]; then
        _ksi_prompt[ps1_suffix]+="\[\e[5 q\]"  # blinking bar cursor
        _ksi_prompt[ps0_suffix]+="\[\e[0 q\]"  # blinking default cursor
    fi

    if [[ "${_ksi_prompt[title]}" == "y" ]]; then
        if [[ -z "$smelly_PID" ]]; then
            if [[ -n "$SSH_TTY" || -n "$SSH2_TTY$smelly_WINDOW_ID" ]]; then
                # connected to most SSH servers
                # or use ssh kitten to connected to some SSH servers that do not set SSH_TTY
                _ksi_prompt[hostname_prefix]="\h: "
            elif [[ -n "$(builtin command -v who)" && "$(builtin command who -m 2> /dev/null)" =~ "\([a-fA-F.:0-9]+\)$" ]]; then
                # the shell integration script is installed manually on the remote system
                # the environment variables are cleared after sudo
                # OpenSSH's sshd creates entries in utmp for every login so use those
                _ksi_prompt[hostname_prefix]="\h: "
            fi
        fi
        # see https://www.gnu.org/software/bash/manual/html_node/Controlling-the-Prompt.html#Controlling-the-Prompt
        # we use suffix here because some distros add title setting to their bashrc files by default
        _ksi_prompt[ps1_suffix]+="\[\e]2;${_ksi_prompt[hostname_prefix]}\w\a\]"
        if [[ "$HISTCONTROL" == *"ignoreboth"* ]] || [[ "$HISTCONTROL" == *"ignorespace"* ]]; then
            _ksi_debug_print "ignoreboth or ignorespace present in bash HISTCONTROL setting, showing running command in window title will not be robust"
        fi
        _ksi_get_current_command() {
            builtin local last_cmd
            last_cmd=$(HISTTIMEFORMAT= builtin history 1)
            last_cmd="${last_cmd#*[[:digit:]]*[[:space:]]}"  # remove leading history number
            last_cmd="${last_cmd#"${last_cmd%%[![:space:]]*}"}"  # remove remaining leading whitespace
            builtin printf "\e]2;%s%s\a" "${_ksi_prompt[hostname_prefix]@P}" "${last_cmd//[[:cntrl:]]}"  # remove any control characters
        }
        _ksi_prompt[ps0_suffix]+='$(_ksi_get_current_command)'
    fi

    if [[ "${_ksi_prompt[mark]}" == "y" ]]; then
        _ksi_prompt[ps1]+="\[\e]133;A\a\]"
        _ksi_prompt[ps2]+="\[\e]133;A;k=s\a\]"
        _ksi_prompt[ps0]+="\[\e]133;C\a\]"
    fi

    builtin alias edit-in-smelly="kitten edit-in-smelly"

    if [[ "${_ksi_prompt[complete]}" == "y" ]]; then
        _ksi_completions() {
            builtin local src
            builtin local limit
            # Send all words up to the word the cursor is currently on
            builtin let limit=1+$COMP_CWORD
            src=$(builtin printf "%s\n" "${COMP_WORDS[@]:0:$limit}" | builtin command kitten __complete__ bash)
            if [[ $? == 0 ]]; then
                builtin eval "${src}"
            fi
        }
        builtin complete -F _ksi_completions smelly
        builtin complete -F _ksi_completions edit-in-smelly
        builtin complete -F _ksi_completions clone-in-smelly
        builtin complete -F _ksi_completions kitten
    fi

    # wrap our prompt additions in markers we can use to remove them using
    # bash's anemic pattern substitution
    if [[ -n "${_ksi_prompt[ps0]}" ]]; then
        _ksi_prompt[ps0]="${_ksi_prompt[start_mark]}${_ksi_prompt[ps0]}${_ksi_prompt[end_mark]}"
    fi
    if [[ -n "${_ksi_prompt[ps0_suffix]}" ]]; then
        _ksi_prompt[ps0_suffix]="${_ksi_prompt[start_suffix_mark]}${_ksi_prompt[ps0_suffix]}${_ksi_prompt[end_suffix_mark]}"
    fi
    if [[ -n "${_ksi_prompt[ps1]}" ]]; then
        _ksi_prompt[ps1]="${_ksi_prompt[start_mark]}${_ksi_prompt[ps1]}${_ksi_prompt[end_mark]}"
    fi
    if [[ -n "${_ksi_prompt[ps1_suffix]}" ]]; then
        _ksi_prompt[ps1_suffix]="${_ksi_prompt[start_suffix_mark]}${_ksi_prompt[ps1_suffix]}${_ksi_prompt[end_suffix_mark]}"
    fi
    if [[ -n "${_ksi_prompt[ps2]}" ]]; then
        _ksi_prompt[ps2]="${_ksi_prompt[start_mark]}${_ksi_prompt[ps2]}${_ksi_prompt[end_mark]}"
    fi
    builtin unset _ksi_prompt[start_mark] _ksi_prompt[end_mark] _ksi_prompt[start_suffix_mark] _ksi_prompt[end_suffix_mark] _ksi_prompt[start_secondary_mark] _ksi_prompt[end_secondary_mark]

    # install our prompt command, using an array if it is unset or already an array,
    # otherwise append a string. We check if _ksi_prompt_command exists as some shell
    # scripts stupidly export PROMPT_COMMAND making it inherited by all programs launched
    # from the shell
    builtin local pc
    pc='builtin declare -F _ksi_prompt_command > /dev/null 2> /dev/null && _ksi_prompt_command'
    if [[ -z "${PROMPT_COMMAND}" ]]; then
        PROMPT_COMMAND=([0]="$pc")
    elif [[ $(builtin declare -p PROMPT_COMMAND 2> /dev/null) =~ 'declare -a PROMPT_COMMAND' ]]; then
        PROMPT_COMMAND+=("$pc")
    else
        builtin local oldval
        oldval=$(builtin shopt -p extglob)
        builtin shopt -s extglob
        PROMPT_COMMAND="${PROMPT_COMMAND%%+([[:space:]])}"
        PROMPT_COMMAND="${PROMPT_COMMAND%%+(;)}"
        builtin eval "$oldval"
        PROMPT_COMMAND+="; $pc"
    fi
    if [ -n "${smelly_IS_CLONE_LAUNCH}" ]; then
        builtin local orig_conda_env="$CONDA_DEFAULT_ENV"
        builtin eval "${smelly_IS_CLONE_LAUNCH}"
        builtin hash -r 2> /dev/null 1> /dev/null
        builtin local venv="${VIRTUAL_ENV}/bin/activate"
        builtin local sourced=""
        _ksi_s_is_ok() {
            [[ -z "$sourced" && "$smelly_CLONE_SOURCE_STRATEGIES" == *",$1,"* ]] && builtin return 0
            builtin return 1
        }

        if _ksi_s_is_ok "venv" && [ -n "${VIRTUAL_ENV}" -a -r "$venv" ]; then
            sourced="y"
            builtin unset VIRTUAL_ENV
            builtin source "$venv"
        fi; if _ksi_s_is_ok "conda" && [ -n "${CONDA_DEFAULT_ENV}" ] && builtin command -v conda >/dev/null 2>/dev/null && [ "${CONDA_DEFAULT_ENV}" != "$orig_conda_env" ]; then
            sourced="y"
            conda activate "${CONDA_DEFAULT_ENV}"
        fi; if _ksi_s_is_ok "env_var" && [[ -n "${smelly_CLONE_SOURCE_CODE}" ]]; then
            sourced="y"
            builtin eval "${smelly_CLONE_SOURCE_CODE}"
        fi; if _ksi_s_is_ok "path" && [[ -r "${smelly_CLONE_SOURCE_PATH}" ]]; then
            sourced="y"
            builtin source "${smelly_CLONE_SOURCE_PATH}"
        fi
        builtin unset -f _ksi_s_is_ok
        # Ensure PATH has no duplicate entries
        if [ -n "$PATH" ]; then
            builtin local old_PATH=$PATH:; PATH=
            while [ -n "$old_PATH" ]; do
                builtin local x
                x=${old_PATH%%:*}
                case $PATH: in
                    *:"$x":*) ;;
                    *) PATH=$PATH:$x;;
                esac
                old_PATH=${old_PATH#*:}
            done
            PATH=${PATH#:}
        fi
    fi
    builtin unset smelly_IS_CLONE_LAUNCH smelly_CLONE_SOURCE_STRATEGIES
}
_ksi_main
builtin unset -f _ksi_main

case :$SHELLOPTS: in
  *:posix:*) ;;
  *)

_ksi_transmit_data() {
    builtin local data
    data="${1//[[:space:]]}"
    builtin local pos=0
    builtin local chunk_num=0
    while [ $pos -lt ${#data} ]; do
        builtin local chunk="${data:$pos:2048}"
        pos=$(($pos+2048))
        builtin printf '\eP@smelly-%s|%s:%s\e\\' "${2}" "${chunk_num}" "${chunk}"
        chunk_num=$(($chunk_num+1))
    done
    # save history so it is available in new shell
    [ "$3" = "save_history" ] && builtin history -a
    builtin printf '\eP@smelly-%s|\e\\' "${2}"
}

clone-in-smelly() {
    builtin local bv="${BASH_VERSINFO[0]}.${BASH_VERSINFO[1]}.${BASH_VERSINFO[2]}"
    builtin local data="shell=bash,pid=$$,bash_version=$bv,cwd=$(builtin printf "%s" "$PWD" | builtin command base64),envfmt=bash,env=$(builtin export | builtin command base64)"
    while :; do
        case "$1" in
            "") break;;
            -h|--help)
                builtin printf "%s\n\n%s\n" "Clone the current bash session into a new smelly window." "For usage instructions see: https://sw.backbiter-no.net/smelly/shell-integration/#clone-shell"
                builtin return
                ;;
            *) data="$data,a=$(builtin printf "%s" "$1" | builtin command base64)";;
        esac
        shift
    done
    _ksi_transmit_data "$data" "clone" "save_history"
}

      ;;
esac

