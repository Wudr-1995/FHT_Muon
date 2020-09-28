# echo "cleanup FhtAnalysis v0 in /junofs/users/wudr/offline20/trunk/Analysis"

if test "${CMTROOT}" = ""; then
  CMTROOT=/cvmfs/juno.ihep.ac.cn/centos7_amd64_gcc830/Pre-Release/J20v1r1-branch/ExternalLibs/CMT/v1r26; export CMTROOT
fi
. ${CMTROOT}/mgr/setup.sh
cmtFhtAnalysistempfile=`${CMTROOT}/mgr/cmt -quiet build temporary_name`
if test ! $? = 0 ; then cmtFhtAnalysistempfile=/tmp/cmt.$$; fi
${CMTROOT}/mgr/cmt cleanup -sh -pack=FhtAnalysis -version=v0 -path=/junofs/users/wudr/offline20/trunk/Analysis  $* >${cmtFhtAnalysistempfile}
if test $? != 0 ; then
  echo >&2 "${CMTROOT}/mgr/cmt cleanup -sh -pack=FhtAnalysis -version=v0 -path=/junofs/users/wudr/offline20/trunk/Analysis  $* >${cmtFhtAnalysistempfile}"
  cmtcleanupstatus=2
  /bin/rm -f ${cmtFhtAnalysistempfile}
  unset cmtFhtAnalysistempfile
  return $cmtcleanupstatus
fi
cmtcleanupstatus=0
. ${cmtFhtAnalysistempfile}
if test $? != 0 ; then
  cmtcleanupstatus=2
fi
/bin/rm -f ${cmtFhtAnalysistempfile}
unset cmtFhtAnalysistempfile
return $cmtcleanupstatus

