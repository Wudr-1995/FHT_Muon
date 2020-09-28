#!/usr/bin/env python

import Sniper
task = Sniper.Task("task")
task.asTop()
task.setLogLevel(0)

import CorAnalisys
alg = task.createAlg("CorAnaAlg/alg_example")

import BufferMemMgr
bufMgr = task.createSvc("BufferMemMgr")

import RootIOSvc
riSvc = task.createSvc("RootInputSvc/InputSvc")
inputFileList = ["sample_detsim.root", "sample_elecsim.root", "sample_calib.root", "sample_rec.root"]

#inputFileList = ["sample_elecsim.root"] # OK
#inputFileList = ["sample_elecsim.root", "sample_calib.root"] # OK
#inputFileList = ["sample_elecsim.root", "sample_calib.root", "sample_rec.root"] # OK

riSvc.property("InputFile").set(inputFileList)

task.setEvtMax(-1)
task.show()
task.run()
