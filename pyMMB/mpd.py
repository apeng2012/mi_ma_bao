import sys
import getpass

import mimabao


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
    pass


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
