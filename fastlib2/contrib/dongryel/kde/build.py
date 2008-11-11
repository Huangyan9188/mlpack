librule(
    name = "kde",                            # this line can be safely omitted
    sources = [],                            # files that must be compiled
    headers = ["kde_problem.h"],
    deplibs = ["fastlib:fastlib_int",
               "contrib/dongryel/multitree_template:multitree_template",
               "contrib/dongryel/proximity_project:proximity_project",
               "mlpack/series_expansion:series_expansion"]
    )

binrule(
    name = "kde_bin",
    sources = ["kde_main.cc"],
    headers = [],
    deplibs = [":kde"]
    )

# to build:
# 1. make sure have environment variables set up:
#    $ source /full/path/to/fastlib/script/fl-env /full/path/to/fastlib
#    (you might want to put this in bashrc)
# 2. fl-build main
#    - this automatically will assume --mode=check, the default
#    - type fl-build --help for help
# 3. ./main
#    - to build same target again, type: make
#    - to force recompilation, type: make clean
