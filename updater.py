import os
import os.path
import tarfile
import fnmatch
import datetime
from shutil import copyfile
from shutil import rmtree
from shutil import move
import subprocess
import signal
import RPi.GPIO as GPIO

UPDATE_DIR = '/home/pi/update'
UPDATE_FILE = UPDATE_DIR + '/update_files.tar.gz'
UPDATE_WORK_FILE = UPDATE_DIR + '/update_files.work.tar.gz'
PI_ROOT_DIR = '/home/pi/pi-i2c'
ROV_ROOT_DIR = '/home/pi'
LOCAL_BIN_DIR = '/usr/local/bin'
LOCAL_LIB_DIR = '/usr/local/lib/mjpg-streamer'
USR_LIB_DIR = '/usr/lib'
USR_LOCAL_LIB_DIR = '/usr/local/lib'
ETC_DIR = '/etc'

# make the update directory
# if it doesn't exist
if os.path.exists(UPDATE_DIR) is False:
    os.mkdir(UPDATE_DIR,0o777)

# check and see if there is an update file
if os.path.exists(UPDATE_FILE) :
    # Set the mode of numbering the pins.
    GPIO.setmode(GPIO.BCM)
    # GPIO pin 12 is an output.
    GPIO.setup(12, GPIO.OUT)
    # turn on the LED flasher
    GPIO.output(12, True)
    # Keep a record of update process and date
    f = open(ROV_ROOT_DIR+"/update.log","a+")
    f.write('System update executed at %s.\n' % (datetime.datetime.now()))

    f.write("Update file exists: " + UPDATE_FILE + "\n")
    # in case the update takes a while change the name
    # of the file so that the update procedure
    # is not triggered again
    os.rename(UPDATE_FILE, UPDATE_WORK_FILE)
    # now we need to kill the cockpit, and streamer processes
    process_strings = ('node src/cockpit.js',
               'rovdrv',
               'openrov-cockpit/src/plugins/mjpeg-video',
               'mjpg_streamer')
    p = subprocess.Popen(['ps', 'aux'], stdout=subprocess.PIPE)
    out, err = p.communicate()
    for line in out.splitlines():
        if any(s in line for s in process_strings):
            f.write(line + "\n")
            pid = int(line.split()[1])
            f.write("Killing PID: " + str(pid) + "\n")
            os.kill(pid, signal.SIGKILL)

    update_contents = UPDATE_WORK_FILE.split(".")[0]
    f.write(update_contents + "\n")
    tarfile.open(UPDATE_WORK_FILE).extractall(update_contents)
    # now change directory to the context of the unpacked file
    os.chdir(update_contents)
    # examine the conntents and determine what actions to take
    for dirpath, dirnames, filenames in os.walk("."):
        dirparts = dirpath.split("/")
        dir = dirparts[len(dirparts)-1]
        if dir == "pi-i2c" : # ends with specific dirname
            # checkout the filenames
            # and copy them to the right place
            # saving off the original
            for filename in filenames:
                source_file = os.path.join(dirpath, filename)
                f.write( source_file  + "\n")
                if filename == "rovdrv" :
                    f.write("Updating " + filename + "\n")
                    replace_file = os.path.join(PI_ROOT_DIR, filename)
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file,replace_file)
                    os.chmod(replace_file, 0o777)
                elif filename == "pin_toggler.py" :
                    f.write("Updating " + filename + "\n")
                    replace_file = os.path.join(ROV_ROOT_DIR, filename)
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file,replace_file)
                    os.chmod(replace_file, 0o777)
                elif filename == "reboot_wifi.sh" :
                    f.write("Updating " + filename + "\n")
                    replace_file = os.path.join(PI_ROOT_DIR, filename)
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file,replace_file)
                    os.chmod(replace_file, 0o777)
                elif filename == "launcher.sh" :
                    f.write("Updating " + filename + "\n")
                    replace_file = os.path.join(ROV_ROOT_DIR, filename)
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file,replace_file)
                    os.chmod(replace_file, 0o777)
                elif filename == "Release_Notes.log" :
                    f.write("Updating " + filename + "\n")
                    replace_file = os.path.join(PI_ROOT_DIR, filename)
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file,replace_file)
                elif filename == "df.fs" :
                    f.write("Updating " + filename + "\n")
                    replace_file = os.path.join(PI_ROOT_DIR, filename)
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file,replace_file)
                    os.chmod(replace_file, 0o777)
                elif filename == "rovconfig.json" :
                    f.write("Updating " + filename + "\n")
                    replace_file = os.path.join(ETC_DIR, filename)
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file,replace_file)
                    os.chmod(replace_file, 0o777)                    
                else :
                    f.write("Unexpected file: " + filename + "\n")
        elif dir == "openrov-cockpit" :
            for filename in filenames:
                source_file = os.path.join(dirpath, filename)
                f.write( os.path.join(dirpath, filename)  + "\n")
                if filename == "openrov-cockpit.tar.gz" :
                    f.write( "Updating " + filename + "\n")
                    replace_file = os.path.join(ROV_ROOT_DIR, filename)
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file,replace_file)
                    # save off the original directory
                    original_dir = os.path.join(ROV_ROOT_DIR, "openrov-cockpit")
                    if os.path.exists(original_dir):
                        if os.path.exists(original_dir+".sv"):
                            rmtree(original_dir+".sv", ignore_errors=True)
                        move(original_dir, original_dir+".sv")
                    # Now unzip the file
                    f.write( "Untar " + replace_file + "\n")
                    tarfile.open(replace_file).extractall(ROV_ROOT_DIR)
                    # Now chmod all the files
                    for root, dirs, files in os.walk(original_dir):  
                        for momo in dirs:  
                            os.chmod(os.path.join(root, momo), 0o777)
                        for momo in files:
                            os.chmod(os.path.join(root, momo), 0o777)
                else :
                    f.write( "Unexpected file: " + filename + "\n")
        elif dir == "mjpg_streamer" :
            # ensure that library directory exists
            if os.path.exists(LOCAL_LIB_DIR) is False:
                os.mkdir(LOCAL_LIB_DIR,0o777)
            
            for filename in filenames:
                source_file = os.path.join(dirpath, filename)
                f.write( source_file  + "\n")
                if filename == "mjpg_streamer" :
                    replace_file = os.path.join(LOCAL_BIN_DIR, filename)
                    f.write( "Updating " + replace_file + "\n")
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file, replace_file)
                    os.chmod(replace_file, 0o777)
                elif filename == "remotecamera.sh":
                    f.write("Updating " + filename + "\n")
                    replace_file = os.path.join(LOCAL_BIN_DIR, filename)
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file + ".sv")
                    copyfile(source_file, replace_file)
                    os.chmod(replace_file, 0o777)
                elif filename == "input_raspicam.so":
                    replace_file = os.path.join(LOCAL_LIB_DIR, filename)
                    f.write( "Updating " + replace_file + "\n")
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file, replace_file)
                    os.chmod(replace_file, 0o777)
                elif filename == "output_file.so":
                    replace_file = os.path.join(LOCAL_LIB_DIR, filename)
                    f.write( "Updating " + replace_file + "\n")
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file, replace_file)
                    os.chmod(replace_file, 0o777)
                elif filename == "output_ws.so":
                    replace_file = os.path.join(LOCAL_LIB_DIR, filename)
                    f.write( "Updating " + replace_file + "\n")
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file, replace_file)
                    os.chmod(replace_file, 0o777)
                elif filename == "output_http.so":
                    replace_file = os.path.join(LOCAL_LIB_DIR, filename)
                    f.write( "Updating " + replace_file + "\n")
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file, replace_file)
                    os.chmod(replace_file, 0o777)
                elif filename == "libuv.so":
                    replace_file = os.path.join(LOCAL_LIB_DIR, filename+".2.0.0")
                    f.write( "Updating " + replace_file + "\n")
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file, replace_file)
                    os.chmod(replace_file, 0o777)
                    base_file = os.path.join(LOCAL_LIB_DIR, filename);
                    if os.path.exists(base_file) : os.remove(base_file)
                    os.symlink(replace_file,base_file)
                    if os.path.exists(base_file+".2") : 
                        os.remove(base_file+".2")
                    os.symlink(replace_file,base_file+".2")
                    # erase other copies
                    base_file = os.path.join(USR_LIB_DIR, filename);
                    if os.path.exists(base_file) : 
                        os.remove(base_file)
                    if os.path.exists(base_file+".2") : 
                        os.remove(base_file+".2")
                    if os.path.exists(base_file+"2.0.0") : 
                        os.remove(base_file+".2.0.0")
                    base_file = os.path.join(USR_LOCAL_LIB_DIR, filename);
                    if os.path.exists(base_file) : 
                        os.remove(base_file)
                    if os.path.exists(base_file+".2") : 
                        os.remove(base_file+".2")
                    if os.path.exists(base_file+"2.0.0") : 
                        os.remove(base_file+".2.0.0")
                elif filename == "libssl.so":
                    replace_file = os.path.join(LOCAL_LIB_DIR, filename+".1.1")
                    f.write( "Updating " + replace_file + "\n")
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file, replace_file)
                    os.chmod(replace_file, 0o777)
                    base_file = os.path.join(LOCAL_LIB_DIR, filename);
                    if os.path.exists(base_file) : 
                        os.remove(base_file)
                    os.symlink(replace_file,base_file)
                    # erase other copies
                    base_file = os.path.join(USR_LIB_DIR, filename);
                    if os.path.exists(base_file) : 
                        os.remove(base_file)
                    if os.path.exists(base_file+".1.1") : 
                        os.remove(base_file+".1.1")
                    base_file = os.path.join(USR_LOCAL_LIB_DIR, filename);
                    if os.path.exists(base_file) : 
                        os.remove(base_file)
                    if os.path.exists(base_file+".1.1") : 
                        os.remove(base_file+".1.1")
                elif filename == "libcrypto.so":
                    replace_file = os.path.join(LOCAL_LIB_DIR, filename+".1.0.0")
                    f.write( "Updating " + replace_file + "\n")
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file, replace_file)
                    os.chmod(replace_file, 0o777)
                    base_file = os.path.join(LOCAL_LIB_DIR, filename);
                    if os.path.exists(base_file) : 
                        os.remove(base_file)
                    os.symlink(replace_file,base_file)
                    # erase other copies
                    base_file = os.path.join(USR_LIB_DIR, filename);
                    if os.path.exists(base_file) : 
                        os.remove(base_file)
                    if os.path.exists(base_file+".1.0.0") : 
                        os.remove(base_file+".1.0.0")
                    base_file = os.path.join(USR_LOCAL_LIB_DIR, filename);
                    if os.path.exists(base_file) : 
                        os.remove(base_file)
                    if os.path.exists(base_file+".1.0.0") : 
                        os.remove(base_file+".1.0.0")
                elif filename == "libuWS.so":
                    replace_file = os.path.join(LOCAL_LIB_DIR, filename)
                    f.write( "Updating " + replace_file + "\n")
                    if os.path.exists(replace_file):
                        os.rename(replace_file, replace_file+".sv")
                    copyfile(source_file, replace_file)
                    os.chmod(replace_file, 0o777)
                    # erase other copies
                    base_file = os.path.join(USR_LIB_DIR, filename);
                    if os.path.exists(base_file) : 
                        os.remove(base_file)
                    base_file = os.path.join(USR_LOCAL_LIB_DIR, filename);
                    if os.path.exists(base_file) : 
                        os.remove(base_file)
                else :
                    f.write( "Unexpected file: " + filename + "\n")
        else :
            f.write( "Unexpected directory: " + dirpath + "\n")

    # now we are done so delete the update file
    os.remove(UPDATE_WORK_FILE)
    # 
    remove_dir = os.path.join(UPDATE_DIR,update_contents)
    rmtree(remove_dir)
    # turn off the LED flasher
    GPIO.output(12, False)
    # close the log file
    f.close()
else :
    print("There is no update file: " + UPDATE_FILE)
