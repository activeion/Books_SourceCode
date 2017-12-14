"""
schedule2.py: traversing OSCON schedule data

    >>> import shelve
    >>> db = shelve.open(DB_NAME)
    >>> if CONFERENCE not in db: load_db(db)


    >>> db.close()

"""

# BEGIN SCHEDULE2_RECORD
import warnings
import inspect  # <1>

import osconfeed

DB_NAME = 'data/schedule2_db'  # <2>
CONFERENCE = 'conference.115'


class Record:
    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)

    def __eq__(self, other):  # <3>
        if isinstance(other, Record):
            return self.__dict__ == other.__dict__
        else:
            return NotImplemented
# END SCHEDULE2_RECORD


# BEGIN SCHEDULE2_DBRECORD
class MissingDatabaseError(RuntimeError):
    """Raised when a database is required but was not set."""  # <1>


class DbRecord(Record):  # <2>

    __db = None  # <3>

    @staticmethod  # <4>
    def set_db(db):
        DbRecord.__db = db  # <5>

    @staticmethod  # <6>
    def get_db():
        return DbRecord.__db

    @classmethod  # <7>
    def fetch(cls, ident):
        db = cls.get_db()
        try:
            return db[ident]  # <8>
        except TypeError:
            if db is None:  # <9>
                msg = "database not set; call '{}.set_db(my_db)'"
                raise MissingDatabaseError(msg.format(cls.__name__))
            else:  # <10>
                raise

    def __repr__(self):
        if hasattr(self, 'serial'):  # <11>
            cls_name = self.__class__.__name__
            return '<{} serial={!r}>'.format(cls_name, self.serial)
        else:
            return super().__repr__()  # <12>

    def __getstat__(self):
        print('aaa')

# END SCHEDULE2_DBRECORD


# BEGIN SCHEDULE2_EVENT
class Event(DbRecord):  # <1>

    @property
    def venue(self):
        key = 'venue.{}'.format(self.venue_serial)
        return self.__class__.fetch(key)  # <2>

    @property
    def speakers(self):
        if not hasattr(self, '_speaker_objs'):  # <3>
            spkr_serials = self.__dict__['speakers']  # <4>
            fetch = self.__class__.fetch  # <5>
            self._speaker_objs = [fetch('speaker.{}'.format(key))
                                  for key in spkr_serials]  # <6>
        return self._speaker_objs  # <7>

    def __repr__(self):
        if hasattr(self, 'name'):  # <8>
            cls_name = self.__class__.__name__
            return '<{} {!r}>'.format(cls_name, self.name)
        else:
            return super().__repr__()  # <9>
# END SCHEDULE2_EVENT


# BEGIN SCHEDULE2_LOAD
def load_db(db):
    raw_data = osconfeed.load()
    warnings.warn('loading ' + DB_NAME)
    for collection, rec_list in raw_data['Schedule'].items():
        record_type = collection[:-1]  # <1> "events" => "event"
        cls_name = record_type.capitalize()  # <2>
        # globals(): Return the dictionary containing the current scope's global variables.
        # dict.get(k, d): D.get(k[,d]) -> D[k] if k in D, else d.  d defaults to None.
        cls = globals().get(cls_name, DbRecord)  # <3>
        if inspect.isclass(cls) and issubclass(cls, DbRecord):  # <4>
            factory = cls  # <5>
        else:
            factory = DbRecord  # <6>
        for record in rec_list:  # <7>
            key = '{}.{}'.format(record_type, record['serial']) # "event.33950"
            record['serial'] = key
            db[key] = factory(**record)  # <8>
# END SCHEDULE2_LOAD

#if __name__ == '__main__':

# BEGIN SCHEDULE1_DEMO
import shelve
db = shelve.open(DB_NAME)  # <1> DB_NAME被shelve调用pickle序列化的文件, db的格式是由pickle决定的
if CONFERENCE not in db:  # <2>
    load_db(db)  # <3>

speaker = db['speaker.3471']  # <4>

print(type(speaker)) # <5> <class 'schedule1.Record'>
print(speaker.name, speaker.twitter) # <6> ('Anna Martelli Ravenscroft', 'annaraven')

# END SCHEDULE1_DEMO

# BEGIN SCHEDULE2_DEMO
DbRecord.set_db(db)  # <1>
event = DbRecord.fetch('event.33950')  # <2>
print(event) # <3> <Event 'There *Will* Be Bugs'>
print(event.venue) # <4> <DbRecord serial='venue.1449'>
print(event.venue.name) # <5> 'Portland 251'
for spkr in event.speakers:  # <6>
    print('{0.serial}: {0.name}'.format(spkr))
''' 
speaker.3471: Anna Martelli Ravenscroft
speaker.5199: Alex Martelli
'''
# END SCHEDULE2_DEMO

db.close()  # <7>
