############################################################################
#   Copyright 2007,2008 Impinj, Inc.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
############################################################################

AR = ${CROSS_COMPILE}ar
CC = ${CROSS_COMPILE}gcc

LTKC_DIR = .
LTKC_LIBS = $(LTKC_DIR)/Library/libltkc.a
LTKC_INCL = -I$(LTKC_DIR)/Library
CFLAGS = -g $(LTKC_INCL)
TARGET = moo_concrete_reader

$(TARGET) : moo_concrete_reader.c
	$(CC) $(CFLAGS) -o moo_concrete_reader moo_concrete_reader.c $(LTKC_LIBS)

clean:
	rm -f $(TARGET) Mon_* Tue_* Wed_* Thu_* Fri_* Sat_* Sun_*

.PHONY: clean
