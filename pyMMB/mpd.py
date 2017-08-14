import os
import sys
import getpass
import termios
import random

import mimabao


def AutoPassword(n):
    ele = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J'\
            'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T'\
            'U', 'V', 'W', 'X', 'Y', 'Z'\
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'\
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j'\
            'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't'\
            'u', 'v', 'w', 'x', 'y', 'z', '!', '@', '#', '$'\
            '%', '^', '&', '*', '-', '+']
    sub = random.sample(ele, n)
    ret = ''.join(sub)
    return ret


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
            if len(_input) == 0:
                continue
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
    while True:
        has_used = False
        setUse = raw_input('This password is used to:')
        cnt = mmb.get_usedto(setUse)
        for x in range(0, cnt):
            if setUse == mmb.get_item():
                has_used = True
                break
        if has_used:
            print setUse + " already exits."
            continue
        else:
            break

    setName = raw_input('Please enter your user name:')

    while True:
        mode = raw_input('Are automatically generated password(y or n):')
        if mode == 'y':
            n = int(raw_input('several char?'))
            setPassword = AutoPassword(n)
        else:
            while True:
                tmp = getpass.getpass('Please enter your user password:')
                setPassword = getpass.getpass('Please enter your user password again:')
                if (tmp == setPassword):
                    break

        #xml.txtToClipboard(setPassword)
        confirm = raw_input('Make sure that the password is available(y or n):')
        if confirm == 'y':
            if mmb.add(setUse, setName, setPassword):
                print "add one MMB item."
                break
            else:
                print "add password item error."


def wait_MMB():
    mou = open("/dev/input/mice", 'rb')
    case = 0
    while (True):
        m = mou.read(3)
        b = ord(m[0])
        if case == 0:
            if ( b & 0x4 ) > 0:
                case += 1
        if case == 1:
            if ( b & 0x4 ) == 0:
                break


def GetPassword(mmb):
    usedto = assist_input(mmb)
    if usedto == '':
        print "input error, or No what you want."
        return

    print mmb.prepare_password(usedto)
    print "In the place where you need to enter the password by the MMB."
    wait_MMB()
    mmb.MMB()
    exit()


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
