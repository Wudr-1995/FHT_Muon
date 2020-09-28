#!/usr/bin/env python

import Sniper
import argparse
def get_parser():
	parser = argparse.ArgumentParser(description='Run Atmospheric Comperason.')
	parser.add_argument("--inputlist", action='append', help="the file which contains input file names")
	# parser.add_argument("--output", default="sim_rec.root", help="output file name")
	parser.add_argument("--user_output", default="sim_rec_user.root", help="user output file name")
	return parser

parser = get_parser();
args = parser.parse_args()

task = Sniper.TopTask("task")
task.setEvtMax(1)
task.setLogLevel(0)

import FhtAnalysis
import RootWriter
rootwriter = task.createSvc("RootWriter")
rootwriter.property("Output").set({"USER_OUTPUT": args.user_output})
alg = task.createAlg("FhtAnalysis/alg_example")

import BufferMemMgr
bufMgr = task.createSvc("BufferMemMgr")

import RootIOSvc
riSvc = task.createSvc("RootInputSvc/InputSvc")
if args.inputlist is None:
	inputdata = ["sample_detsim.root", "sample_elecsim.root", "sample_calib.root", "sample_rec.root"]
else:
	inputdata = []
	for infile in args.inputlist:
		print(infile)
		inputdata.append(infile)
riSvc.property("InputFile").set(inputdata)

# roSvc = task.createSvc("RootOutputSvc/OutputSvc")
# outputdata = {"/Event/Sim_Rec": args.output}
# roSvc.property("OutputStreams").set(outputdata)

# inputFileList = ["sample_elecsim.root"] # OK
# inputFileList = ["sample_elecsim.root", "sample_calib.root"] # OK
# inputFileList = ["sample_elecsim.root", "sample_calib.root", "sample_rec.root"] # OK

task.setEvtMax(-1)
task.show()
task.run()
