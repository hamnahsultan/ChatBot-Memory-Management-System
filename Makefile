# ============================================
# CS200 Chatbot Memory Manager - Makefile
# ============================================

CC      = gcc
CFLAGS  = -Wall -Wextra -O0 -g
INCLUDE = -Iinclude

SRC_DIR   = src
TEST_DIR  = tests

# ==========================================
# SOURCE FILES
# ============================================

PARSER    = $(SRC_DIR)/parser.c
MESSAGE   = $(SRC_DIR)/message.c
HISTORY   = $(SRC_DIR)/history.c
COMPACT   = $(SRC_DIR)/compaction.c

CORE_SRC  = $(PARSER) $(MESSAGE) $(HISTORY) $(COMPACT)

# ============================================
# TEST FILES
# ============================================

TEST_PARSER   = $(TEST_DIR)/test_parser.c
TEST_MESSAGE  = $(TEST_DIR)/test_message.c
TEST_HISTORY  = $(TEST_DIR)/test_history.c
TEST_COMPACT  = $(TEST_DIR)/test_compaction.c
TEST_FULL     = $(TEST_DIR)/chatbot_demo.c

# ============================================
# OUTPUT BINARIES
# ============================================

BIN_PARSER   = test_parser
BIN_MESSAGE  = test_message
BIN_HISTORY  = test_history
BIN_COMPACT  = test_compaction
BIN_FULL     = chatbot_demo

# ============================================
# DEFAULT
# ============================================

all: help

# ============================================
# HELP
# ============================================

help:
	@echo ""
	@echo "CS200 Test Harness Makefile"
	@echo "---------------------------"
	@echo "make parser     -> Run parser tests"
	@echo "make message    -> Run message tests"
	@echo "make history    -> Run history tests"
	@echo "make compaction -> Run compaction tests"
	@echo "make full       -> Run full system demo"
	@echo ""
	@echo "make v-history  -> History tests with Valgrind"
	@echo "make v-full     -> Full demo with Valgrind"
	@echo ""
	@echo "make clean      -> Remove all binaries"
	@echo ""

# ============================================
# PARSER
# ============================================

parser:
	$(CC) $(CFLAGS) \
	$(PARSER) $(TEST_PARSER) \
	$(INCLUDE) \
	-o $(BIN_PARSER)
	./$(BIN_PARSER)

# ============================================
# MESSAGE
# ============================================

message:
	$(CC) $(CFLAGS) \
	$(MESSAGE) $(TEST_MESSAGE) \
	$(INCLUDE) \
	-o $(BIN_MESSAGE)
	./$(BIN_MESSAGE)

# ============================================
# HISTORY
# ============================================

history:
	$(CC) $(CFLAGS) \
	$(HISTORY) $(MESSAGE) $(COMPACT) $(PARSER) \
	$(TEST_HISTORY) \
	$(INCLUDE) \
	-o $(BIN_HISTORY)
	./$(BIN_HISTORY)

# ============================================
# COMPACTION
# ============================================

compaction:
	$(CC) $(CFLAGS) \
	$(HISTORY) $(MESSAGE) $(COMPACT) $(PARSER) \
	$(TEST_COMPACT) \
	$(INCLUDE) \
	-o $(BIN_COMPACT)
	./$(BIN_COMPACT)

# ============================================
# FULL SYSTEM DEMO
# ============================================

full:
	$(CC) $(CFLAGS) \
	$(CORE_SRC) $(TEST_FULL) \
	$(INCLUDE) \
	-o $(BIN_FULL)
	./$(BIN_FULL)

# ============================================
# VALGRIND (OPTION 2: EXIT CODE)
# ============================================

v-history: history
	valgrind --error-exitcode=1 --leak-check=full ./$(BIN_HISTORY)

v-full: full
	valgrind --error-exitcode=1 --leak-check=full ./$(BIN_FULL)

# ============================================
# CLEAN
# ============================================

clean:
	rm -f \
	$(BIN_PARSER) \
	$(BIN_MESSAGE) \
	$(BIN_HISTORY) \
	$(BIN_COMPACT) \
	$(BIN_FULL)