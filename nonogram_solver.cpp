#include <bits/stdc++.h>
using namespace std;
#define rep(i,n) for (int i = 0; i < (n); i++)

class NonogramDescription;
class NonogramDrawer;
class NonogramSolution;
class Solver;
class BitArray;

enum CellState{
    EMPTY,
    FILLED,
    NOT_DECIDED
};
// ================================================================================================================================

class BitArray {
private:
    int length;
    int chunks;
    vector<long long> bits;
public:
    BitArray() {}
    BitArray(int len) {
        length = len;
        chunks = (len - 1) / 64 + 1;
        bits = vector<long long>(chunks, 0);
    }
    bool getBit(int index) {
        assert(index < length);
        return ((bits[index/64] & (1LL<<(index%64))) != 0);
    }
    void setBit (int index, bool value) {
        assert(index < length);
        long long indicator = (1LL<<(index%64));
        if (value) {
            bits[index/64] |= indicator;
        } else if ((bits[index/64] & indicator) != 0) {
            bits[index/64] -= indicator;
        }
    }
    void andWith(BitArray a) {
        assert(a.length == length);
        rep(i, chunks) {
            bits[i] &= a.bits[i];
        }
    }
    void orWith(BitArray a) {
        assert(a.length == length);
        rep(i, chunks) {
            bits[i] |= a.bits[i];
        }
    }
};

// ================================================================================================================================

class NonogramDescription {
private:
    int width;
    int height;
    vector<vector<int>> columnDescriptions;
    vector<vector<int>> rowDescriptions;

public:
    NonogramDescription() {

    }
    NonogramDescription(string file) {
        ifstream in(file);
        cin.rdbuf(in.rdbuf());

        cin >> height >> width;
        rowDescriptions.resize(height);
        columnDescriptions.resize(width);
        rep(i,height) {
            int k;
            cin >> k;
            rowDescriptions[i].resize(k);
            rep(j,k) cin >> rowDescriptions[i][j];
        }
        rep(i,width) {
            int k;
            cin >> k;
            columnDescriptions[i].resize(k);
            rep(j,k) cin >> columnDescriptions[i][j];
        }
    }

    int getWidth() { return width; }
    int getHeight() { return height; }
    vector<int> getRowDescriptions(int y) { return rowDescriptions[y]; }
    vector<int> getColumnDescriptions(int x) { return columnDescriptions[x]; }
};

// ================================================================================================================================

class NonogramSolution {
public:
    char EMPTY_MARKER = ' ';
    char FILL_MARKER = '#';

private:
    int width;
    int height;
    vector<vector<bool>> pixels;
public:
    bool impossible = false;;
    NonogramSolution() {impossible = true;}
    NonogramSolution(vector<vector<CellState>> cells) {
        width = cells.size();
        height = cells[0].size();
        pixels = vector<vector<bool>>(width, vector<bool>(height));
        rep(x, width) rep(y, height) {
            pixels[x][y] = (cells[x][y] == CellState::FILLED);
        }
    }
    string toString() {
        string s = "";
        rep(y, height) {
            rep(x, width) {
                s += (pixels[x][y] ? FILL_MARKER : EMPTY_MARKER);
            }
            if (y != height - 1) s += "\n";
        }
        return s;
    }
};

// ================================================================================================================================

class Solver {
public:
    enum SolveResult {
        SOLVED,
        AMBIGUOUS,
        IMPOSSIBLE,
        NOT_ATTEMPTED
    };
private:
    int width;
    int height;
    NonogramDescription descr;
    vector<vector<CellState>> cells;
    SolveResult solveResult;

    vector<CellState> curRow;
    BitArray curRowBits;
    BitArray curRowFilled;
    BitArray curRowNotEmpty;
    int curRowLength;
    vector<int> curDescription;
    bool curRowImpossible;

    bool verbose = true;
public:
    Solver(NonogramDescription desc) {
        width = desc.getWidth();
        height = desc.getHeight();
        // cerr << width << " " << height << endl;
        descr = desc;
        cells =  vector<vector<CellState>>(width, vector<CellState>(height, CellState::NOT_DECIDED));
        solveResult = SolveResult::NOT_ATTEMPTED;
    }

    NonogramSolution solve() {
        if (solveRec() == SolveResult::IMPOSSIBLE) {
            solveResult = SolveResult::IMPOSSIBLE;
            return NonogramSolution(); // 本当は null を返したい
        } else {
            solveResult = SolveResult::SOLVED;
            return NonogramSolution(cells); 
        }
    }

private:
    void printTime (clock_t start) {
        double time = 1.0 * (clock() - start) / CLOCKS_PER_SEC;
        if (time > 10) cout << "Time        : " << time  << " (sec)" << endl;
    }
    bool solveRow(vector<CellState> &row, vector<int> desc) {
        clock_t start = clock();
        if (verbose) {
            string s = "Solving row: ";
            for (int x : desc) {
                s += to_string(x) + " ";
            }
            s += "\n";
            s += "Before      : ";
            for (CellState cs : row) {
                switch (cs)
                {
                case CellState::EMPTY:
                    s += '.';
                    break;
                case CellState::FILLED:
                    s += '#';
                    break;
                case CellState::NOT_DECIDED:
                    s += '?';
                    break;
                default:
                    break;
                }
            }
            cout << s << endl;
        }

        curRow = row;
        curDescription = desc;
        curRowLength = row.size();
        curRowFilled = BitArray(curRowLength);
        curRowNotEmpty = BitArray(curRowLength);
        rep(x, curRowLength) {
            curRowFilled.setBit(x, true);
        }
        curRowBits = BitArray(curRowLength);

        if (!solveRowRec(0, 0)) {
            curRowImpossible = true;
            printTime(start);
            return false;
        }

        bool changed = false;
        rep(x, curRowLength) {
            if (row[x] == CellState::NOT_DECIDED) {
                if (curRowFilled.getBit(x)) {
                    row[x] = CellState::FILLED;
                    changed = true;
                } else if (!curRowNotEmpty.getBit(x)) {
                    row[x] = CellState::EMPTY;
                    changed = true;
                }
            }
        }

        if (changed && verbose) {
            string s = "After       : ";
            for (CellState cs : row) {
                switch (cs)
                {
                case CellState::EMPTY:
                    s += '.';
                    break;
                case CellState::FILLED:
                    s += '#';
                    break;
                case CellState::NOT_DECIDED:
                    s += '?';
                    break;
                default:
                    break;
                }
            }
            cout << s << endl;
        }
        printTime(start);
        return changed;
    }

    bool solveRowRec(int pos, int numsUsed) {
        if (numsUsed == curDescription.size()) {
            assert(pos <= curRowLength);
            for (int x = pos; x < curRowLength; x++) {
                if (curRow[x] == CellState::FILLED) return false;
            }

            curRowFilled.andWith(curRowBits);
            curRowNotEmpty.orWith(curRowBits);

            return true;
        }

        int curLen = curDescription[numsUsed];

        // Go through all possible starts.
        bool feasible = false;
        for (int x = pos; x < curRowLength; x++) {
            if (x + curLen > curRowLength) break;

            // Check that it doesn't have empty cells inside.
            bool canBeFilled = true;
            for (int i = x; i < x + curLen; i++) {
                if (curRow[i] == CellState::EMPTY) {
                    canBeFilled = false;
                    break;
                }
            }

            // Check that right after his group cell can be empty.
            if (canBeFilled && x + curLen < curRowLength && curRow[x + curLen] == CellState::FILLED) {
                canBeFilled = false;
            }

            if (canBeFilled) {
                for (int i = x; i < x + curLen; i++) {
                    curRowBits.setBit(i, true);
                }
                int emptyCell = ((x + curLen == curRowLength) ? 0 : 1);
                if (solveRowRec(x + curLen + emptyCell, numsUsed + 1)) {
                    feasible = true;
                }
                for (int i = x; i < x + curLen; i++) {
                    curRowBits.setBit(i, false);
                }
            }

            if (curRow[x] == CellState::FILLED) break;
        }

        return feasible;
    }

    bool solveStep() {
        bool changed = false;
        
        // Check all rows.
        rep(y, height) {
            vector<CellState> row(width);
            rep(x, width) row[x] = cells[x][y];
            if (solveRow(row, descr.getRowDescriptions(y))) {
                changed = true;
                rep(x, width) {
                    cells[x][y] = row[x];
                }
            }
            if (curRowImpossible) return false;
        }

        // Check all columns.
        rep(x,width) {
            vector<CellState> col(height);
            rep(y, height) col[y] = cells[x][y];
            if (solveRow(col, descr.getColumnDescriptions(x))) {
                changed = true;
                rep(y, height) {
                    cells[x][y] = col[y];
                }
            }
            if (curRowImpossible) return false;
        }

        return changed;
    }
    
    SolveResult solveRec() {
        curRowImpossible = false;
        int limit = 1000;
        int loopCount = 0;
        while (limit--) {
            loopCount++;
            // cerr << "solveRec while" << endl;
            bool changed = solveStep();
            if (curRowImpossible) return SolveResult::IMPOSSIBLE;
            if (!changed) break;
        }
        cout << "loopCount : " << loopCount << endl;

        bool solved = true;
        int badX = 0, badY = 0;
        rep(x, width) {
            rep(y, height) {
                if (cells[x][y] == CellState::NOT_DECIDED) {
                    solved = false;
                    badX = x;
                    badY = y;
                    break;
                }
            }
        }

        if (solved) return SolveResult::SOLVED;

        // Go deeper.
        vector<vector<CellState>> backupCells(width, vector<CellState>(height));
        rep(x, width) rep(y, height) backupCells[x][y] = cells[x][y];

        cells[badX][badY] = CellState::EMPTY;
        if (solveRec() == SolveResult::SOLVED) {
            return SolveResult::SOLVED;
        } else {
            cells = backupCells;
            cells[badX][badY] = CellState::FILLED;
            return solveRec();
        }
    }
};

// ================================================================================================================================

void solve(string fileName) {
    clock_t start = clock();

    NonogramDescription desc;
    try {
        desc = NonogramDescription("in/" + fileName + "_in.txt");
    } catch (exception& e) {
        cerr << "error" << endl;
        return;
    }
    Solver solver = Solver(desc);
    NonogramSolution sol = solver.solve();

    if (sol.impossible) {
        cout << "No solution." << endl;
        return;
    }

    string solAscii = sol.toString();
    cout << solAscii << endl;

    ofstream ofs{"out/" + fileName + "_out.txt"};
    ofs << solAscii << endl;

    clock_t finish = clock();
    double time = 1.0 * (finish - start) / CLOCKS_PER_SEC;
    if (time < 1) cout << "Time : " << time * 1000 << " (ms)" << endl;
    else if (time < 60) cout << "Time : " << time << " (sec)" << endl;
    else cout << "Time : " << time / 60 << " (min)" << endl;
}

// ================================================================================================================================

int main() {
    string fileName;
    cin >> fileName;
    solve(fileName);
}