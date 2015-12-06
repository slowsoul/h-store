CC = gcc
CXX = g++ -std=gnu++0x
CFLAGS = -g -O2 -fPIC -DDEBUG_SLP -DSL_DEBUG
MEMMGR = -ltcmalloc_minimal

all: sl_test sl_compact_test slp_test bloomfilter_test

sl_test.o: sl_test.cc skiplist_map.h skiplist_traits.h
	$(CXX) $(CFLAGS) -c -o $@ $<

sl_multimap_test.o: sl_multimap_test.cc skiplist_multimap.h skiplist_traits.h
	$(CXX) $(CFLAGS) -c -o $@ $<

sl_compact_test.o: sl_compact_test.cc skiplist_map.h skiplist_map_compact.h skiplist_traits.h
	$(CXX) $(CFLAGS) -c -o $@ $<

sl_compact_merge_test.o: sl_compact_merge_test.cc skiplist_map.h skiplist_map_compact.h skiplist_traits.h
	$(CXX) $(CFLAGS) -c -o $@ $<

sl_multimap_compact_test.o: sl_multimap_compact_test.cc skiplist_multimap.h skiplist_multimap_ro.h skiplist_multimap_compact.h skiplist_traits.h
	$(CXX) $(CFLAGS) -c -o $@ $<

sl_multimap_compact_merge_test.o: sl_multimap_compact_merge_test.cc skiplist_multimap.h skiplist_multimap_ro.h skiplist_traits.h
	$(CXX) $(CFLAGS) -c -o $@ $<

slp_test.o: slp_test.cc slp.h
	$(CXX) $(CFLAGS) -c -o $@ $<

bloomfilter_test.o: bloomfilter_test.cc bloomfilter.h
	$(CXX) $(CFLAGS) -c -o $@ $<

sl_test: sl_test.o
	$(CXX) $(CFLAGS) -o $@ $< $(MEMMGR) -lpthread -lm

sl_multimap_test: sl_multimap_test.o
	$(CXX) $(CFLAGS) -o $@ $< $(MEMMGR) -lpthread -lm

sl_compact_test: sl_compact_test.o
	$(CXX) $(CFLAGS) -o $@ $< $(MEMMGR) -lpthread -lm

sl_compact_merge_test: sl_compact_merge_test.o
	$(CXX) $(CFLAGS) -o $@ $< $(MEMMGR) -lpthread -lm

sl_multimap_compact_test: sl_multimap_compact_test.o
	$(CXX) $(CFLAGS) -o $@ $< $(MEMMGR) -lpthread -lm

sl_multimap_compact_merge_test: sl_multimap_compact_merge_test.o
	$(CXX) $(CFLAGS) -o $@ $< $(MEMMGR) -lpthread -lm

slp_test: slp_test.o
	$(CXX) $(CFLAGS) -o $@ $< $(MEMMGR) -lpthread -lm

bloomfilter_test: bloomfilter_test.o
	$(CXX) $(CFLAGS) -o $@ $< $(MEMMGR) -lpthread -lm

clean:
	rm -f *.o sl_test sl_multimap_test sl_compact_test sl_compact_merge_test sl_multimap_compact_test sl_multimap_compact_merge_test slp_test bloomfilter_test
