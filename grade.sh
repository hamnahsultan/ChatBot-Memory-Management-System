#!/bin/bash

echo ""
echo "=============================================="
echo "   CS200 CHATBOT — FINAL AUTOGRADING REPORT"
echo "=============================================="
echo ""

TOTAL=0

# -----------------------------------------------
# Utility: run test, count PASS/FAIL, weight score
# -----------------------------------------------
run_and_score () {
    NAME="$1"
    WEIGHT="$2"
    CMD="$3"

    echo "▶ Running: $NAME"
    OUTPUT=$($CMD 2>&1)

    PASS=$(echo "$OUTPUT" | grep -c "\[PASS\]")
    FAIL=$(echo "$OUTPUT" | grep -c "\[FAIL\]")

    if [ $((PASS + FAIL)) -eq 0 ]; then
        SCORE=0
    else
        SCORE=$(awk "BEGIN { printf \"%.1f\", ($PASS/($PASS+$FAIL))*$WEIGHT }")
    fi

    TOTAL=$(awk "BEGIN { printf \"%.1f\", $TOTAL + $SCORE }")
    printf "%-40s %6.1f / %2d\n" "$NAME" "$SCORE" "$WEIGHT"
    echo ""
}

# -----------------------------------------------
# Utility: binary integration check
# -----------------------------------------------
run_binary () {
    NAME="$1"
    WEIGHT="$2"
    CMD="$3"

    echo "▶ Running: $NAME"
    if $CMD > /dev/null 2>&1; then
        SCORE=$WEIGHT
    else
        SCORE=0
    fi

    TOTAL=$(awk "BEGIN { printf \"%.1f\", $TOTAL + $SCORE }")
    printf "%-40s %6.1f / %2d\n" "$NAME" "$SCORE" "$WEIGHT"
    echo ""
}

# -----------------------------------------------
# Utility: Valgrind via exit code (OPTION 2)
# -----------------------------------------------
run_valgrind () {
    NAME="$1"
    WEIGHT="$2"
    CMD="$3"

    echo "▶ Running: $NAME (Valgrind)"

    if $CMD > /tmp/vg.log 2>&1; then
        # Perfect: no leaks, no errors
        SCORE=$WEIGHT
        NOTE="clean"
    else
        # Valgrind failed — check if program still ran
        if grep -q "ERROR SUMMARY" /tmp/vg.log; then
            # Memory issues, but program executed
            SCORE=$(awk "BEGIN { printf \"%.1f\", $WEIGHT * 0.4 }")
            NOTE="leaks"
        else
            # Crash or severe failure
            SCORE=0
            NOTE="crash"
        fi
    fi

    TOTAL=$(awk "BEGIN { printf \"%.1f\", $TOTAL + $SCORE }")
    printf "%-40s %6.1f / %2d   (%s)\n" "$NAME" "$SCORE" "$WEIGHT" "$NOTE"
    echo ""
}

# ===============================================
# GRADING CATEGORIES (FINAL WEIGHTS)
# ===============================================

run_and_score "Message Layer Correctness"          10 "make message"
run_and_score "Parser & Moderation"                16 "make parser"

run_and_score "History Invariants & Deletion"      20 "make history"
run_and_score "Compaction & Fragmentation"         15 "make compaction"

run_and_score "Policy Enforcement & Purge"         10 "make history"
run_and_score "Stress & Robustness (Unit)"          8 "make history"

run_binary   "Integration Stability (Demo)"         8 "make full"

run_valgrind "Memory Safety (History)"              6 "make v-history"
run_valgrind "Memory Safety (Integration)"          7 "make v-full"

# ===============================================
# FINAL SUMMARY
# ===============================================

echo "----------------------------------------------"
printf "%-40s %6.1f / 100\n" "TOTAL SCORE" "$TOTAL"
echo "----------------------------------------------"
echo ""

exit 0