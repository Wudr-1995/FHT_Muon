#!/bin/bash


scriptdir=$JUNOTOP/offline/Examples/Tutorial/share

function regenerate_bkg() {
    BKGDIR=/junofs/production/validation/J17v1r1-Pre2/tao/BKG-LS-500k-detsim/uniform

    python $ROOTIOTOOLSROOT/share/merge.py U238-500k.root $BKGDIR/U-238/detsim/detsim-500*.root
    python $ROOTIOTOOLSROOT/share/merge.py Th232-500k.root $BKGDIR/Th-232/detsim/detsim-501*.root
    python $ROOTIOTOOLSROOT/share/merge.py K40-500k.root $BKGDIR/K-40/detsim/detsim-502*.root
}

function chain_elec() {
    local dir=$FUNCNAME
    echo $dir
    #run detsim
    #python $scriptdir/tut_detsim.py --evtmax 10 hepevt --exe IBD

    #run elecsim
    python $scriptdir/tut_det2elec.py --evtmax -1 --loop 0

    #run calibration
    python $scriptdir/tut_elec2calib.py --evtmax -1

    #run reconstruction
    python $scriptdir/tut_calib2rec.py --evtmax -1
}

function chain_mixing() {
    local dir=$FUNCNAME
    echo $dir
    [ -d "$dir" ] || mkdir $dir
    [ -d "$dir" ] || exit -1

    BKGDIR=$(pwd)


    cd $dir

    #python $scriptdir/tut_detsim.py --evtmax 10 hepevt --exe IBD

    #BKGDIR=/junofs/production/validation/J17v1r1-Pre2/tao/BKG-LS-500k-detsim/uniform
    # gdb --args \
    python $scriptdir/tut_det2elec.py --evtmax -1 \
        --input sample_detsim.root --rate 1000000 \
        --input U:$BKGDIR/U238-500k.root --rate U:6 \
        --input Th:$BKGDIR/Th232-500k.root --rate Th:6 \
        --input K:$BKGDIR/K40-500k.root --rate K:3


    #run calibration
    # gdb --args \
    python $scriptdir/tut_elec2calib.py --evtmax -1

    #run reconstruction
    python $scriptdir/tut_calib2rec.py --evtmax -1

    python run.py
}

$*