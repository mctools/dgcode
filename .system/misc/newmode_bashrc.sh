function dgbuild2()
{
    real_dgbuild2_cmd=$(which unwrapped_dgbuild2) || ( echo "dgbuild2: command not found" 1>&2 ; exit 1 )
    "${real_dgbuild2_cmd}" "$@" && eval "$(${real_dgbuild2_cmd} --env-setup)"
    unset real_dgbuild2_cmd
}
