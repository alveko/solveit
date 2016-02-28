# simple in-memory key-value database
#     it has two maps: key->value and value->value_counter
#
# operations:
#     set(key, value)
#     unset(key)
#     get(key)
#     countEqualTo(value)
#
# simple support for transactions:
#     begin()    - start a new transaction (can be nested)
#     rollback() - rollback the most recent (nested) transaction
#     commit()   - commit all started transactions
#
# the implementation below uses hash maps, thus, O(1) on average for the ops
# if a BST map is used instead, it will be O(logN)

import sys
from collections import namedtuple
from collections import defaultdict


class SimpleDatabase:

    class Transaction:

        Change = namedtuple('Change', ['key', 'oldvalue', 'newvalue'])

        def __init__(self, sdb, auto_commit=False):
            self.changes = []
            self._sdb = sdb
            self._auto_commit = auto_commit

        def __enter__(self):
            return self

        def __exit__(self, type, value, traceback):
            if self._auto_commit:
                self.commit()

        def commit(self):
            for change in self.changes:
                if change.oldvalue is not None:
                    self._sdb._db_cnt[change.oldvalue] -= 1
                if change.newvalue is not None:
                    self._sdb._db[change.key] = change.newvalue
                    self._sdb._db_cnt[change.newvalue] += 1
                else:
                    self._sdb._db.pop(change.key, None)

    def __init__(self):
        self._db = defaultdict(lambda: None)
        self._db_cnt = defaultdict(int)
        self._transaction_log = []

    def _get_transaction(self):
        if self._transaction_log:
            # if there is an ongoing transaction in the log, return it
            return self._transaction_log[-1]
        else:
            # no ongoing transaction, create a temporary one
            # (changes will be committed as soon as it's destroyed)
            return self.Transaction(sdb=self, auto_commit=True)

    def set(self, key, value):
        with self._get_transaction() as transaction:
            transaction.changes.append(self.Transaction.Change(
                key=key, oldvalue=self._db[key], newvalue=value))

    def unset(self, key):
        self.set(key, None)

    def get(self, key):
        value = self._db[key]
        # check not yet committed changes (supposed to be few)
        for transaction in self._transaction_log:
            for change in transaction.changes:
                if key == change.key:
                    value = change.newvalue
        return value

    def count_equal_to(self, value):
        cnt = self._db_cnt.get(value, 0)
        # check not yet committed changes (supposed to be few)
        for transaction in self._transaction_log:
            for change in transaction.changes:
                if change.newvalue == value:
                    cnt += 1
                if change.oldvalue == value:
                    cnt -= 1
        return cnt

    def begin(self):
        self._transaction_log.append(self.Transaction(sdb=self))

    def rollback(self):
        if not self._transaction_log:
            return False
        self._transaction_log.pop()
        return True

    def commit(self):
        if not self._transaction_log:
            return False
        for transaction in self._transaction_log:
            transaction.commit()
        self._transaction_log.clear()
        return True

if __name__ == "__main__":

    # for simplicity, no validation of the input commands
    class SimpleDatabaseCli:
        def __init__(self):
            self.sdb = SimpleDatabase()

        def set(self, args):
            self.sdb.set(*args)

        def unset(self, args):
            self.sdb.unset(*args)

        def get(self, args):
            value = self.sdb.get(*args)
            if value:
                print(str(value))
            else:
                print("NULL")

        def numequalto(self, args):
            print(str(self.sdb.count_equal_to(*args)))

        def begin(self, args):
            self.sdb.begin(*args)

        def rollback(self, args):
            if not self.sdb.rollback(*args):
                print("NO TRANSACTION")

        def commit(self, args):
            if not self.sdb.commit(*args):
                print("NO TRANSACTION")

        def run(self):
            while True:
                line = sys.stdin.readline().strip()
                args = line.split(' ')
                # print("# " + line)

                fn = getattr(SimpleDatabaseCli, args[0].lower(), None)
                if fn:
                    fn(self, args[1:])
                elif len(args[0]):
                    break

    SimpleDatabaseCli().run()
