#!/usr/bin/python

# sudo apt-get install python-crypto

import os, sys
from random import randrange
from Crypto.Cipher import Blowfish

# This script reads in the apps-whitelist.txt file and generates
# a data structure that can be pased into ApplicationManager.cpp. 

kApps = "\
com.palm.launcher:\
com.palm.systemui:\
\
com.palm.app.amazonstore:\
com.palm.app.backup:\
com.palm.app.bluetooth:\
com.palm.app.browser:\
com.palm.app.calculator:\
com.palm.app.calendar:\
com.palm.app.camera:\
com.palm.app.certificate:\
com.palm.app.contacts:\
com.palm.app.dataimport:\
com.palm.app.dateandtime:\
com.palm.app.deviceinfo:\
com.palm.app.devmodeswitcher:\
com.palm.app.docviewer:\
com.palm.app.email:\
com.palm.app.facebook:\
com.palm.app.firstuse:\
com.palm.app.help:\
com.palm.app.languagepicker:\
com.palm.app.location:\
com.palm.app.maps:\
com.palm.app.messaging:\
com.palm.app.musicplayer:\
com.palm.app.notes:\
com.palm.app.pdfviewer:\
com.palm.app.phone:\
com.palm.app.phoneprefs:\
com.palm.app.photos:\
com.palm.app.screenlock:\
com.palm.app.soundsandalerts:\
com.palm.app.sprintportal:\
com.palm.app.streamingmusicplayer:\
com.palm.app.tasks:\
com.palm.app.updates:\
com.palm.app.videoplayer:\
com.palm.app.videoplayer.launcher:\
com.palm.app.wifi:\
com.palm.app.youtube:\
\
com.handson.app.nascar:\
com.telenav.app.sprintnavigation:\
com.mobitv.app.sprinttv:\
\
\x00";


# Blowfish cipher needs 8 byte blocks to work with
def __pad_file( file_buffer):
    pad_bytes = 8 - (len(file_buffer) % 8)                                 
    for i in range(pad_bytes - 1): file_buffer += chr(randrange(0, 256))
    # final padding byte; % by 8 to get the number of padding bytes
    bflag = randrange(6, 248); bflag -= bflag % 8 - pad_bytes
    file_buffer += chr(bflag)
    return file_buffer

def writefile(outfile_name, file_buffer):
    outfile = open(outfile_name, 'wb')
    outfile.write(file_buffer)
    outfile.close()

# Paul says rot13 is enough for our purposes.
def rot13( inStr ):
    outStr = ""
    for x in range(len(inStr)):
        byte = ord(inStr[x])
        cap = (byte & 32)
        byte = (byte & (~cap))
        if (byte >= ord('A')) and (byte <= ord('Z')):
            byte = ((byte - ord('A') + 13) % 26 + ord('A'))
        byte = (byte | cap)
        outStr = str( outStr + (chr(byte)) )
    return outStr

def main():
    #result = "\n"
    #for appid in allowedApps:
    #    result = str( result+"\""+rot13(appid)+"\", // "+appid+"\n" )
    #print result
    
    #Blowflish
    key="Copyright 2009 Palm Inc."
    filename = "/usr/lib/lib_id.so"
    __cipher = Blowfish.new(key,Blowfish.MODE_CBC)
    result = __cipher.encrypt(__pad_file(kApps))
    
    writefile( "/tmp/apps", result );
    
    print str( "File written to " + "/tmp/apps" )
    print str("Key is \"" + rot13(key)+ "\"")
    print str("filename is \"" + rot13(filename)+ "\" ("+filename+")")
    #print "// Paste this into ApplicationManager.cpp, \"ApplicationManager::checkAppAgainstWhiteList\""


main()


