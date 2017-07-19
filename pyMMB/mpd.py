import os
import sys
import getpass
import termios

import mimabao


def assist_input(mmb):
    fd = sys.stdin.fileno()
    old_ttyinfo = termios.tcgetattr(fd)
    new_ttyinfo = old_ttyinfo[:]
    new_ttyinfo[3] &= ~termios.ICANON
    new_ttyinfo[3] &= ~termios.ECHO
    termios.tcsetattr(fd, termios.TCSANOW, new_ttyinfo)

    os.system('clear')
    _input = ''
    cnt = 0
    first_item = ''
    while True:
        ch = os.read(fd, 1)
        if ch == '\n':
            if cnt == 0:
                first_item = ''
            break
        if ch == '\x7F': #EDL
            _input = _input[:-1]
        else:
            _input += ch

        os.system('clear')
        print _input

        cnt = mmb.get_usedto(_input)

        for x in range(0, cnt):
            if x == 0:
                first_item = mmb.get_item()
                print first_item
            else:
                print mmb.get_item()

    termios.tcsetattr(fd, termios.TCSANOW, old_ttyinfo)
    return first_item


def InitPermit(mmb):
    password1 = getpass.getpass("This MMB has not yet been used. Set password:")
    password2 = getpass.getpass("Enter password again:")
    if password1 != password2:
        print "password error!"
        sys.exit()
    mmb.set_permit(password1)


def SetPassword(mmb):
    pass


def GetPassword(mmb):
    usedto = assist_input(mmb)
    if usedto == '':
        print "input error, or No what you want."
        return

    print mmb.prepare_password(usedto)
    print "In the place where you need to enter the password by the MMB."



def Backup(mmb):
    pass


if __name__ == '__main__':

    mmb = mimabao.MiMaBao()

    mmb.status();
    if mmb.st_isinitpermit() != True:
        InitPermit(mmb)
        print "Run MMB again."
        sys.exit()
    if mmb.st_ispermit() != True:
        password = getpass.getpass('Please enter the password:')
        if mmb.permit(password) != True:
            print 'Sorry , wrong password!'
            sys.exit()


    while True:

        item = raw_input(\
'''
        1: GetPassword.
        2: SetPassword.
        3: Backup.
        4: Exit.
''')
        if item == '1':
            GetPassword(mmb)
        elif item == '2':
            SetPassword(mmb)
        elif item == '3':
            BackupPassword(mmb)
        else:
            sys.exit()
