function dgbuild2()
{
    which unwrapped_dgbuild2 || ( echo "dgbuild2: command not found" 1>&2 ; exit 1 )
    unwrapped_dgbuild2 "$@" && eval "$(unwrapped_dgbuild2 --env-setup)"
}
