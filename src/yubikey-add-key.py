#!/usr/bin/env python
import sys
import sqlite3

db, user, uid, aes = sys.argv[1:]


conn = sqlite3.connect(db)
c = conn.cursor()
c.execute('insert into tokens (user, uid, aes, counter, session) values (?,?,?,0,0)',
          (user, uid, aes))
conn.commit()
c.close()
