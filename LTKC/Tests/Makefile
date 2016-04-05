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
LTKC_LIBS =../Library/libltkc.a
LTKC_INCL = -I ../Library
CFLAGS = -g $(LTKC_INCL)
TARGET = dx201

$(TARGET) : dx201.c
	$(CC) -o dx201 dx201.c $(LTKC_LIBS) $(LTKC_INCL)

clean:
	rm -f $(TARGET) Mon_* Tue_* Wed_* Thu_* Fri_* Sat_* Sun_*
	rm -rf *.o

.PHONY: clean
