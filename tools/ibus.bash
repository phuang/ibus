# bash completion for ibus

if ! type _get_comp_words_by_ref >/dev/null 2>&1; then
if [[ -z ${ZSH_VERSION:+set} ]]; then
_get_comp_words_by_ref ()
{
    local exclude cur_ words_ cword_
    if [ "$1" = "-n" ]; then
        exclude=$2
        shift 2
    fi
    __git_reassemble_comp_words_by_ref "$exclude"
    cur_=${words_[cword_]}
    while [ $# -gt 0 ]; do
        case "$1" in
        cur)
            cur=$cur_
            ;;
        prev)
            prev=${words_[$cword_-1]}
            ;;
        words)
            words=("${words_[@]}")
            ;;
        cword)
            cword=$cword_
            ;;
        esac
        shift
    done
}
else
_get_comp_words_by_ref ()
{
    while [ $# -gt 0 ]; do
        case "$1" in
        cur)
            cur=${COMP_WORDS[COMP_CWORD]}
            ;;
        prev)
            prev=${COMP_WORDS[COMP_CWORD-1]}
            ;;
        words)
            words=("${COMP_WORDS[@]}")
            ;;
        cword)
            cword=$COMP_CWORD
            ;;
        -n)
            # assume COMP_WORDBREAKS is already set sanely
            shift
            ;;
        esac
        shift
    done
}
fi
fi

__ibus()
{
    COMPREPLY=()

    local cur_=$2 prev_=$3 cur words cword prev
    _get_comp_words_by_ref -n =: cur words cword prev

    # echo
    # echo "cur='$cur'"
    # echo "prev='$prev'"
    # echo "words='${words[@]}'"
    # echo "cwords='${cwords[@]}'"

    # Commands
    local cmds=( engine list-engine watch )

    local i c cmd subcmd
    for (( i=1; i < ${#words[@]}-1; i++)) ; do
        [[ -n $cmd ]] && subcmd=${words[i]} && break
        for c in ${cmds[@]}; do
            [[ ${words[i]} == $c ]] && cmd=$c && break
        done
    done

    case $cmd in
        engine)
            if [[ "$cmd" == "$prev" ]]; then
                local imes=`ibus list-engine --name-only`
                COMPREPLY=( $( compgen -W '$imes' -- "$cur" | sed 's/^$cur/$cur_/' ))
            fi
            return 0
            ;;
        list-engine)
            if [[ "$cur" == -* ]]; then
                local options=( --name-only )
                COMPREPLY=( $( compgen -W '${options[@]}' -- "$cur" ))
            fi
            return 0
            ;;
        watch)
            return 0
            ;;
    esac

    COMPREPLY=( $( compgen -W '${cmds[@]}' -- "$cur" ))

} &&
complete -o bashdefault -o default -o nospace -F __ibus ibus
