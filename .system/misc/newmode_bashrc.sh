function dgbuild2()
{
    real_dgbuild2_cmd=$(which unwrapped_dgbuild2 2>/dev/null) || ( echo "error: dgbuild not available" 1>&2 ; exit 1 )
    if [ $? == 0 -a "x${real_dgbuild2_cmd}" != "x" ]; then
        "${real_dgbuild2_cmd}" "$@" && eval "$(${real_dgbuild2_cmd} --env-setup)"
    fi
    unset real_dgbuild2_cmd
}
