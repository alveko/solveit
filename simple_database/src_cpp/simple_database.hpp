#ifndef SIMPLE_DATABASE_HPP
#define SIMPLE_DATABASE_HPP

#include <cassert>
#include <list>
#include <memory>
#include <map>
#include <unordered_map>

//! simple in-memory key-value database
//!     it has two maps: key->value and value->value_counter
//!
//! operations:
//!     set(key, value)
//!     unset(key)
//!     get(key)
//!     countEqualTo(value)
//!
//! simple support for transactions:
//!     begin()    - start a new transaction (can be nested)
//!     rollback() - rollback the most recent (nested) transaction
//!     commit()   - commit all started transactions
//!
//! SimpleDatabase is a generic template; can be used with any map container
//! SimpleDatabaseHash - all operations are O(1) on average
//! SimpleDatabaseTree - all operations are O(logN)

namespace SimpleDatabase {

template<typename Key, typename Value, Value NullValue,
         template<typename, typename> class Map>
class SimpleDatabase {
public:

    virtual void set(const Key &key, const Value &value) {
        getTransaction()->changes.push_back({key, get_from_db(key), value});
    }

    virtual void unset(const Key &key) {
        getTransaction()->changes.push_back({key, get_from_db(key), NullValue});
    }

    virtual Value get(const Key &key) {
        Value value = get_from_db(key);
        // check not yet committed changes (supposed to be few)
        transaction_log.foreach([&](const Change &change) {
            if (change.key == key) {
                value = change.newvalue;
            }
        });
        return value;
    }

    virtual size_t countEqualTo(const Value &value) {
        if (value == NullValue) {
            return 0;
        }
        size_t cnt = db_cnt.count(value) ? db_cnt[value] : 0;
        // check not yet committed changes (supposed to be few)
        transaction_log.foreach([&](const Change &change) {
            if (change.newvalue == value) {
                cnt++;
            }
            if (change.oldvalue == value) {
                assert(cnt > 0);
                cnt--;
            }
        });
        return cnt;
    }

    virtual void begin() {
        transaction_log.transactions.push_back(
            std::make_shared<Transaction>(*this));
    }

    virtual bool rollback() {
        if (transaction_log.transactions.empty()) {
            return false;
        }
        // clear all changes from the last transaction
        // so that they don't get committed
        transaction_log.transactions.back()->changes.clear();
        transaction_log.transactions.pop_back();
        return true;
    }

    virtual bool commit() {
        if (transaction_log.transactions.empty()) {
            return false;
        }
        // pop all transactions in chronological order
        // (changes are applied as the transactions gets destroyed)
        while (!transaction_log.transactions.empty()) {
            transaction_log.transactions.pop_front();
        }
        return true;
    }

protected:

    struct Change {
        Key key;
        Value oldvalue;
        Value newvalue;
    };

    class Transaction {
    private:
        SimpleDatabase &sdb;
    public:
        std::list<Change> changes;

        Transaction(SimpleDatabase &sdb) : sdb(sdb) { }

        virtual ~Transaction() {
            // transaction is being destroyed
            // => commit pending changes
            for (const auto &change : changes) {
                if (change.oldvalue != NullValue) {
                    assert(sdb.db_cnt[change.oldvalue] > 0);
                    sdb.db_cnt[change.oldvalue]--;
                }
                if (change.newvalue != NullValue) {
                    sdb.db[change.key] = change.newvalue;
                    sdb.db_cnt[change.newvalue]++;
                } else {
                    sdb.db.erase(change.key);
                }
            }
        }
    };

    using TransactionPtr = std::shared_ptr<Transaction>;

    class TransactionLog {
    public:
        void foreach(std::function<void(const Change &change)> fn) {
            for (auto transaction : transactions) {
                for (const auto &change : transaction->changes) {
                    fn(change);
                }
            }
        }
        std::list<TransactionPtr> transactions;
    };

    TransactionPtr getTransaction() {
        if (!transaction_log.transactions.empty()) {
            // there is an ongoing transaction in the log, return it
            return transaction_log.transactions.back();
        } else {
            // no ongoing transaction, create a temporary one
            // (changes will be committed as soon as it's destroyed)
            return std::make_shared<Transaction>(*this);
        }
    }

    Value get_from_db(const Key& key) {
        return db.count(key) ? db[key] : NullValue;
    }

private:
    Map<Key, Value> db;
    Map<Value, size_t> db_cnt;
    TransactionLog transaction_log;
};

template<typename Key, typename Value>
using HashMap = std::unordered_map<Key, Value, std::hash<Key>,
    std::equal_to<Key>, std::allocator<std::pair<const Key, Value>>>;

template<typename Key, typename Value>
using TreeMap = std::map<Key, Value, std::less<Value>,
    std::allocator<std::pair<const Key, Value>>>;

template<typename Key, typename Value, Value NullValue = Value()>
using SimpleDatabaseHash = SimpleDatabase<Key, Value, NullValue, HashMap>;

template<typename Key, typename Value, Value NullValue = Value()>
using SimpleDatabaseTree = SimpleDatabase<Key, Value, NullValue, TreeMap>;

}

#endif //SIMPLE_DATABASE_HPP
