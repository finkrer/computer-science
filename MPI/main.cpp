#include <fstream>
#include <vector>
#include <mpi.h>

const int ITERATION_COUNT = 5;

const int DEAD = 0;
const int ALIVE = 1;

std::pair<std::vector<int>, std::pair<int, int>> readBoard(const int &rank)
{
    std::vector<int> board;
    int rows, cols;
    if (rank == 0)
    {
        std::ifstream in("input.txt");
        in >> rows >> cols;
        board.resize(rows * cols);
        for (int i = 0; i < rows * cols; ++i)
        {
            in >> board[i];
        }
    }
    return std::make_pair(board, std::make_pair(rows, cols));
}

void initializeProcesses(std::vector<int> &board, int &rows, int &cols, const int &rank)
{
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0)
    {
        board = std::vector<int>(rows * cols);
    }
}

std::pair<std::vector<int>, std::vector<int>> initializeSizes(const int &rows, const int &cols, const int &rank, const int &size)
{
    std::vector<int> counts(size), starts(size);
    for (int i = 0; i < size; ++i)
    {
        counts[i] = (rows / size + (i < rows % size ? 1 : 0)) * cols;
        starts[i] = (i > 0 ? starts[i - 1] + counts[i - 1] : 0);
    }
    return std::make_pair(counts, starts);
}

void shareBoard(std::vector<int> &localBoard, std::vector<int> &board, std::vector<int> &counts,
                std::vector<int> &starts, const int &rows, const int &cols, const int &rank, const int &size)
{
    MPI_Scatterv(board.data(), counts.data(), starts.data(), MPI_INT,
                 localBoard.data(), counts[rank], MPI_INT, 0, MPI_COMM_WORLD);
}

void shareHaloRows(std::vector<int> &localBoard, std::vector<int> &board, std::vector<int> &counts,
                   std::vector<int> &starts, std::vector<int> &haloUp, std::vector<int> &haloDown,
                   const int &localRows, const int &cols, const int &rank, const int &size)
{
    if (rank == 0)
    {
        std::fill(haloUp.begin(), haloUp.end(), DEAD);
    }
    if (rank == size - 1)
    {
        std::fill(haloDown.begin(), haloDown.end(), DEAD);
    }

    if (rank != 0)
    {
        MPI_Recv(haloUp.data(), cols, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    if (rank != size - 1)
    {
        MPI_Send(localBoard.data() + (localRows - 1) * cols, cols, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }
    if (rank != size - 1)
    {
        MPI_Recv(haloDown.data(), cols, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    if (rank != 0)
    {
        MPI_Send(localBoard.data(), cols, MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
    }
}

void gatherBoard(std::vector<int> &localBoard, std::vector<int> &board, std::vector<int> &counts,
                 std::vector<int> &starts, const int &rows, const int &cols, const int &rank, const int &size)
{
    if (rank == 0)
    {
        board.resize(rows * cols);
    }
    MPI_Gatherv(localBoard.data(), counts[rank], MPI_INT,
                board.data(), counts.data(), starts.data(), MPI_INT, 0, MPI_COMM_WORLD);
}

void saveBoard(std::vector<int> &board, const int &rows, const int &cols, const int &rank)
{
    if (rank == 0)
    {
        std::ofstream out("output.txt");
        out << rows << " " << cols << std::endl;
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                out << board[i * cols + j] << " ";
            }
            out << std::endl;
        }
    }
}

int getCellStatus(std::vector<int> &localBoard, std::vector<int> &haloUp, std::vector<int> &haloDown, int i, int j, int rows, int cols)
{
    if (i == -1)
    {
        return (j < 0 || j >= cols) ? DEAD : haloUp[j];
    }
    else if (i == rows)
    {
        return (j < 0 || j >= cols) ? DEAD : haloDown[j];
    }
    else
    {
        return (j < 0 || j >= cols || i < 0 || i > rows) ? DEAD : localBoard[i * cols + j];
    }
}

void tick(std::vector<int> &localBoard, std::vector<int> &haloUp, std::vector<int> &haloDown,
          const int &rows, const int &cols)
{
    std::vector<int> newBoard = localBoard;
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            int aliveNeighbors = 0;
            for (int di = -1; di <= 1; ++di)
            {
                for (int dj = -1; dj <= 1; ++dj)
                {
                    if (di == 0 && dj == 0)
                        continue;
                    aliveNeighbors += getCellStatus(localBoard, haloUp, haloDown, i + di, j + dj, rows, cols);
                }
            }
            if (localBoard[i * cols + j] == ALIVE)
            {
                newBoard[i * cols + j] = (aliveNeighbors == 2 || aliveNeighbors == 3) ? ALIVE : DEAD;
            }
            else
            {
                newBoard[i * cols + j] = (aliveNeighbors == 3) ? ALIVE : DEAD;
            }
        }
    }
    localBoard = newBoard;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    auto configuration = readBoard(rank);
    std::vector<int> &board = configuration.first;
    int rows = configuration.second.first;
    int cols = configuration.second.second;

    initializeProcesses(board, rows, cols, rank);

    auto sizeConfiguration = initializeSizes(rows, cols, rank, size);
    std::vector<int> &counts = sizeConfiguration.first;
    std::vector<int> &starts = sizeConfiguration.second;

    std::vector<int> localBoard(counts[rank]);
    shareBoard(localBoard, board, counts, starts, rows, cols, rank, size);

    std::vector<int> haloUp(cols), haloDown(cols);

    int localRows = counts[rank] / cols;

    for (int i = 0; i < ITERATION_COUNT; i++)
    {
        shareHaloRows(localBoard, board, counts, starts, haloUp, haloDown, localRows, cols, rank, size);
        tick(localBoard, haloUp, haloDown, localRows, cols);
    }

    gatherBoard(localBoard, board, counts, starts, rows, cols, rank, size);

    saveBoard(board, rows, cols, rank);
    MPI_Finalize();

    return 0;
}