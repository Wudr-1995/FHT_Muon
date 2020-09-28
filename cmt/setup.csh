# echo "setup FhtAnalysis v0 in /junofs/users/wudr/offline20/trunk/Analysis"

if ( $?CMTROOT == 0 ) then
  setenv CMTROOT /cvmfs/juno.ihep.ac.cn/centos7_amd64_gcc830/Pre-Release/J20v1r1-branch/ExternalLibs/CMT/v1r26
endif
source ${CMTROOT}/mgr/setup.csh
set cmtFhtAnalysistempfile=`${CMTROOT}/mgr/cmt -quiet build temporary_name`
if $status != 0 then
  set cmtFhtAnalysistempfile=/tmp/cmt.$$
endif
${CMTROOT}/mgr/cmt setup -csh -pack=FhtAnalysis -version=v0 -path=/junofs/users/wudr/offline20/trunk/Analysis  -no_cleanup $* >${cmtFhtAnalysistempfile}
if ( $status != 0 ) then
  echo "${CMTROOT}/mgr/cmt setup -csh -pack=FhtAnalysis -version=v0 -path=/junofs/users/wudr/offline20/trunk/Analysis  -no_cleanup $* >${cmtFhtAnalysistempfile}"
  set cmtsetupstatus=2
  /bin/rm -f ${cmtFhtAnalysistempfile}
  unset cmtFhtAnalysistempfile
  exit $cmtsetupstatus
endif
set cmtsetupstatus=0
source ${cmtFhtAnalysistempfile}
if ( $status != 0 ) then
  set cmtsetupstatus=2
endif
/bin/rm -f ${cmtFhtAnalysistempfile}
unset cmtFhtAnalysistempfile
exit $cmtsetupstatus

