TEMPLATE = subdirs

# usage:
# qmake
# qmake "COMPONENTS = mountainview"

#COMPONENTS = mdaconvert  mountainprocess mountainview prv

isEmpty(COMPONENTS) {
    COMPONENTS = mda mdaconvert mountainprocess mountainsort mountainview mountaincompare prv mountainsort2 mountainsort3
}

CONFIG += ordered

SUBDIRS += cpp/mlconfig/src/mlconfig.pro
SUBDIRS += cpp/mlcommon/src/mlcommon.pro
SUBDIRS += cpp/mvcommon/src/mvcommon.pro

defineReplace(ifcomponent) {
  contains(COMPONENTS, $$1) {
    message(Enabling $$1)
    return($$2)
  }
  return("")
}

SUBDIRS += $$ifcomponent(mdaconvert,cpp/mdaconvert/src/mdaconvert.pro)
SUBDIRS += $$ifcomponent(mda,cpp/mda/src/mda.pro)
SUBDIRS += $$ifcomponent(mountainbrowser,cpp/mountainbrowser/src/mountainbrowser.pro)
SUBDIRS += $$ifcomponent(mountainprocess,cpp/mountainprocess/src/mountainprocess.pro)
SUBDIRS += $$ifcomponent(mountainsort,cpp/mountainsort/src/mountainsort.pro)
SUBDIRS += $$ifcomponent(mountainview,cpp/mountainview/src/mountainview.pro)
SUBDIRS += $$ifcomponent(mountaincompare,cpp/mountaincompare/src/mountaincompare.pro)
SUBDIRS += $$ifcomponent(prv,cpp/prv/src/prv.pro)
SUBDIRS += $$ifcomponent(mountainview-eeg,packages/mountainlab-eeg/mountainview-eeg/src/mountainview-eeg.pro)
SUBDIRS += $$ifcomponent(mountainsort2,packages/mountainsort2/src/mountainsort2.pro)
SUBDIRS += $$ifcomponent(mountainsort3,packages/mountainsort3/src/mountainsort3.pro)
SUBDIRS += $$ifcomponent(sslongview,packages/sslongview/src/sslongview.pro)



deb.target = deb
deb.commands = debuild $(DEBUILD_OPTS) -us -uc

QMAKE_EXTRA_TARGETS += deb