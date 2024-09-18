#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

# define MAX_SIZE 100 // Maximum size for matrices and lists


/* Template class for Node structure
Each node contains a value (data), a row, a column, and a pointer that points to the next node in the sequence*/
template <typename T>
class Node
{
public:
    T data;
    int row;
    int col;
    Node<T> *next;

    //Constructor to initialize the memeber variables
    Node(T data, int row, int col) : data(data), row(row), col(col), next(nullptr) {}
};


/*Template class for a circularly linked list which is the basis to create our matricies using column and row wise linked lists*/
template <typename T>
class CircularLinkedList
{
private:
    Node<T> *head;

public:
    //Constructor to initialize empty linkedlist with head = nullptr
    CircularLinkedList() : head(nullptr) {}

    /*Funtion to insert a new node at with a given value and position
    If the list is empty then the node is added at the start and links back to itself, otherwise the node is added to the end of the list and is linked back to the head*/
    void insert(T value, int row, int col)
    {
        Node<T> *newNode = new Node<T>(value, row, col); //Create the node with the value and posiition
        //This handles the case if there's nothing in the linked list
        if (head == nullptr)
        {
            head = newNode;
            head->next = head;
        }
        else //loops through the list and adds the new node at the end of the list
        {
            Node<T> *temp = head;
            while (temp->next != head)
            {
                temp = temp->next;
            }
            temp->next = newNode;
            newNode->next = head; //new node points back to head
        }
    }
    //Accessor to return the head of the list
    Node<T> *getHead() const { return head; }


    /*OMITTED Function used for debugging printing all the nodes in the list
    void display()
    {
        if (head == nullptr)
            return;
        Node<T> *temp = head;
        do
        {
            std::cout << "(" << temp->row << "," << temp->col << "): " << temp->data << " -> ";
            temp = temp->next;
        } while (temp != head);
        std::cout << "HEAD" << std::endl;
    }*/
};


/*Class to represent a sparse matrix using circular linked lists for row-wise and column-wise storage
Each matrix is represented by two arrays of circular linked lists: one for rows and one for columns*/
class Matrix
{
public:
    CircularLinkedList<int> rows[MAX_SIZE];
    CircularLinkedList<int> cols[MAX_SIZE];
    int size;

    //Constructor to create matrix
    //The size determines the rows and columns
    Matrix(int s) : size(s) {}

    //Function to insert a value into the matrix at a specific row and column
    //The value is stored in both the row-wise and column-wise linked lists
    void insertValue(int row, int col, int value)
    {
        rows[row - 1].insert(value, row, col);
        cols[col - 1].insert(value, row, col);
    }

    //Function to retrieve a value from the matrix at a specific row and column
    int getValue(int row, int col) const
    {
        Node<int> *current = rows[row - 1].getHead();
        if (current == nullptr)
            return 0;
        //Traverse linkedlist till the correct value is found
        do
        {
            if (current->col == col)
            {
                return current->data;
            }
            current = current->next;
        } while (current != rows[row - 1].getHead());
        return 0;
    }

    /*OMITTED Function used to print contents of linked list
    void display()
    {
        std::cout << "Row-wise Circular Linked Lists:" << std::endl;
        for (int i = 0; i < size; ++i)
        {
            std::cout << "Row " << (i + 1) << ": ";
            rows[i].display();
        }
    }*/

};



class MatrixCSVReader
{
private:
    Matrix* matrices[MAX_SIZE];
    int matrixCount;
    char operation;
    int size;
    int scalarMultiplier;

    void parseHeader(const std::string &line)
    {
        std::istringstream ss(line);
        std::string token;

        std::getline(ss, token, ',');
        operation = token[0];

        std::getline(ss, token, ',');
        size = std::stoi(token);
    }

    Matrix addMatrices(const Matrix &a, const Matrix &b)
    {
        Matrix result(size);
        for (int i = 1; i <= size; ++i)
        {
            for (int j = 1; j <= size; ++j)
            {
                int sum = a.getValue(i, j) + b.getValue(i, j);
                if (sum != 0)
                {
                    result.insertValue(i, j, sum);
                }
            }
        }
        return result;
    }

    Matrix multiplyMatrices(const Matrix &a, const Matrix &b)
    {
        Matrix result(size);
        for (int i = 1; i <= size; ++i)
        {
            for (int j = 1; j <= size; ++j)
            {
                int sum = 0;
                for (int k = 1; k <= size; ++k)
                {
                    sum += a.getValue(i, k) * b.getValue(k, j);
                }
                if (sum != 0)
                {
                    result.insertValue(i, j, sum);
                }
            }
        }
        return result;
    }

    Matrix transposeMatrix(const Matrix &m)
    {
        Matrix result(size);
        for (int i = 1; i <= size; ++i)
        {
            for (int j = 1; j <= size; ++j)
            {
                int value = m.getValue(i, j);
                if (value != 0)
                {
                    result.insertValue(j, i, value);
                }
            }
        }
        return result;
    }

    Matrix scalarMultiply(const Matrix &m)
    {
        Matrix result(size);
        for (int i = 1; i <= size; ++i)
        {
            for (int j = 1; j <= size; ++j)
            {
                int value = m.getValue(i, j) * scalarMultiplier;
                if (value != 0)
                {
                    result.insertValue(i, j, value);
                }
            }
        }
        return result;
    }

public:
    MatrixCSVReader() : matrixCount(0) {}

    char getOperation() { return operation; }

    void readCSV(const std::string &filename)
    {
        std::ifstream file(filename);
        std::string line;

        if (!file.is_open())
        {
            std::cerr << "Error opening file: " << filename << std::endl;
            return;
        }

        if (std::getline(file, line))
        {
            parseHeader(line);
        }

        Matrix *currentMatrix = nullptr;

        while (std::getline(file, line))
        {
            if (line.empty() || line == ",,")
            {
                if (currentMatrix)
                {
                    matrices[matrixCount++] = currentMatrix;
                }
                currentMatrix = new Matrix(size);
                continue;
            }

            std::istringstream ss(line);
            std::string token;
            int values[3];
            int valueCount = 0;

            while (std::getline(ss, token, ','))
            {
                if (!token.empty())
                {
                    values[valueCount++] = std::stoi(token);
                }
            }

            if (valueCount == 3 && currentMatrix)
            {
                currentMatrix->insertValue(values[0], values[1], values[2]);
            }
            else if (valueCount == 1 && operation == 'S')
            {
                scalarMultiplier = values[0];
            }
        }

        if (currentMatrix)
        {
            matrices[matrixCount++] = currentMatrix;
        }

        file.close();
    }

    void processMatrices()
    {
        if (matrixCount == 0)
            return;

        Matrix result = *matrices[0];

        switch (operation)
        {
        case 'A':
            for (int i = 1; i < matrixCount; i++)
            {
                result = addMatrices(result, *matrices[i]);
            }
            break;
        case 'M':
            for (int i = 1; i < matrixCount; i++)
            {
                result = multiplyMatrices(result, *matrices[i]);
            }
            break;
        case 'T':
            result = transposeMatrix(result);
            break;
        case 'S':
            result = scalarMultiply(result);
            break;
        }

        matrixCount = 1;
        matrices[0] = new Matrix(result);
    }

    void writeToCSV(const std::string &filename)
    {
        std::ofstream file(filename);
        if (!file.is_open())
        {
            std::cerr << "Error opening file for writing: " << filename << std::endl;
            return;
        }

        Matrix *matrix = matrices[0];
        for (int i = 1; i <= size; ++i)
        {
            for (int j = 1; j <= size; ++j)
            {
                int value = matrix->getValue(i, j);
                if (value != 0)
                {
                    file << i << "," << j << "," << value << std::endl;
                }
            }
        }

        file.close();
        std::cout << "Output written to " << filename << std::endl;
    }
};

int main()
{
    MatrixCSVReader reader;
    std::string inputfile;
    std::cout << "What is the name of the input file? (without the .csv)" << std::endl;
    std::cin >> inputfile;
    reader.readCSV(inputfile + ".csv");
    reader.processMatrices();
    reader.writeToCSV(inputfile + "_output.csv");

    return 0;
}
