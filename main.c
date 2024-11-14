#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <float.h>

#define MAX 512
#define INFINITY FLT_MAX
#define BUSLIST_FILE "BusNumber.csv"

typedef struct Node
{
    char *data;
    struct Node *next;
} Node;

typedef struct stationL
{
    char bnum[MAX];
    char sname[MAX][MAX];
    int stationLen;
} StationL;

typedef struct adj
{
    char bnum[MAX];
    float adjM[MAX][MAX];
    float matrixLen;
} AdjM;

typedef struct
{
    int stop;
    float weight;
    int bus;
} Edge;

typedef struct
{
    int vertex;
    float distance;
    int bus;
} VertexDistancePair;

int busLen;
int stationLen;
StationL stationList[MAX];
AdjM adjMatrix[MAX];
Node *busNum = NULL;
Node *allStation = NULL;
Edge graph[MAX][MAX];

void insert(Node **head, char *data)
{
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->data = (char *)malloc(strlen(data));
    strcpy(new_node->data, data);
    Node *ptr;
    ptr = *head;
    if (*head == NULL)
    {
        *head = new_node;
    }
    else
    {
        while (ptr->next != NULL)
        {
            ptr = ptr->next;
        }
        ptr->next = new_node;
    }
}

int list_size(Node *head)
{
    int count = 0;
    Node *current = head;
    while (current != NULL)
    {
        count++;
        current = current->next;
    }
    return count;
}

// Read Bus Station from File and store in linked list
void getBusStationFromFile()
{
    busNum = NULL;
    allStation = NULL;
    char *filename = BUSLIST_FILE;
    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        printf("Error: could not open file %s", filename);
    }
    char row[MAX];
    while (fgets(row, MAX, fp))
    {
        row[strcspn(row, "\r\n")] = 0;
        insert(&busNum, row);
    }

    fclose(fp);
    busLen = list_size(busNum);
    Node *ptr = busNum;
    int bCount = 0;
    while (ptr != NULL)
    {
        FILE *file = fopen(strcat(ptr->data, ".csv"), "r");
        int bRow = 0;
        while (fgets(row, MAX, file) != NULL)
        {
            row[strcspn(row, "\r\n")] = 0;
            char *tok = strtok(row, ",");
            if (bRow == 0)
            {
                strcpy(stationList[bCount].bnum, tok);
                strcpy(adjMatrix[bCount].bnum, tok);
            }

            int col = 0;
            while (tok != NULL)
            {
                if (bRow == 0 && col > 0)
                {
                    strcpy(stationList[bCount].sname[col - 1], tok);
                }
                else if (col > 0)
                {
                    adjMatrix[bCount].adjM[bRow - 1][col - 1] = atof(tok);
                }
                tok = strtok(NULL, ",");
                col++;
            }
            bRow++;
            stationList[bCount].stationLen = col - 1;
            adjMatrix[bCount].matrixLen = col - 1;
        }
        bCount++;
        fclose(file);
        ptr = ptr->next;
    }
}

// This code finds the station that has the same name as the station
// passed in as a parameter. If the station is found, then it returns
// 1, otherwise it returns 0.
int findRepeatStation(Node *head, char *station)
{
    Node *ptr = head;
    while (ptr != NULL)
    {
        if (!strcmp(ptr->data, station))
        {
            return 1;
        }
        ptr = ptr->next;
    }
    return 0;
}

void createUnionStation()
{
    for (int i = 0; i < busLen; i++)
    {
        for (int j = 0; j < stationList[i].stationLen; j++)
        {
            if (!findRepeatStation(allStation, stationList[i].sname[j]))
            {
                insert(&allStation, stationList[i].sname[j]);
            }
        }
    }
    stationLen = list_size(allStation);
}

// This code adds an edge to the graph.
// It takes in the start and end vertices as well as the weight and the bus.
// If there is already an edge between the two vertices,
// it will only add the edge if the new weight is less than the old weight.
// If there is no edge, it will add it.
// It does not add the edge if the new weight is greater than the old weight.
void addEdge(int start, int end, float weight, int bus)
{
    Edge newEdge;
    newEdge.stop = end;
    newEdge.weight = weight;
    newEdge.bus = bus;
    if (graph[start][end].weight != -1 && weight < graph[start][end].weight)
    {
        graph[start][end] = newEdge;
    }
    else if (graph[start][end].weight == -1)
    {
        graph[start][end] = newEdge;
    }
}

// This function finds the smallest distance from the start node to a node that has not been visited yet.
// It does this by iterating over all the nodes in the graph,
// and if the node has not been visited and the distance to the node is less than the current minimum distance
//, it saves the node as the new minimum distance node.
// It returns the node with the minimum distance that has not been visited yet.
int getMinDistanceStop(bool visited[], float distances[])
{
    float minDistance = INFINITY;
    float minStop = -1;

    for (int i = 0; i < stationLen; i++)
    {
        if (!visited[i] && distances[i] < minDistance)
        {
            minDistance = distances[i];
            minStop = i;
        }
    }

    return minStop;
}

// This code converts a station ID to the station name.
char *convertStationIDToStationName(int n)
{
    Node *ptr = allStation;
    for (int i = 0; i < n; i++)
    {
        ptr = ptr->next;
    }
    return ptr->data;
}

// This function prints the path from the source to the destination
void printPath(int parents[], int destination, int *buses)
{
    int check = 0;
    if (check == 0)
        printf("Path following : ");
    if (parents[destination] == -1)
    {
        printf("%s ", convertStationIDToStationName(destination));
        return;
    }
    check = 1;
    printPath(parents, parents[destination], buses);
    printf("➤ %s [ Bus %d ] ", convertStationIDToStationName(destination), buses[destination]);
}

// This code finds the station ID in the linked list of stations
// The function accepts a station name as a parameter
// The function returns the ID of the station in the linked list
int findStationID(char *station)
{
    Node *ptr = allStation;
    int n = 0;
    while (ptr != NULL)
    {
        if (!strcmp(ptr->data, station))
        {
            break;
        }
        ptr = ptr->next;
        n++;
    }
    return n;
}

void dijkstra(char *s, char *d)
{
    int source = findStationID(s);
    int destination = findStationID(d);
    bool visited[stationLen];
    for (int i = 0; i < stationLen; i++)
    {
        visited[i] = false;
    }
    float distances[stationLen];
    int parents[stationLen];
    int buses[stationLen];

    for (int i = 0; i < stationLen; i++)
    {
        distances[i] = INFINITY;
        parents[i] = -1;
        buses[i] = -1;
    }

    distances[source] = 0;

    for (int i = 0; i < stationLen - 1; i++)
    {
        int currentStop = getMinDistanceStop(visited, distances);
        visited[currentStop] = true;

        for (int j = 0; j < stationLen; j++)
        {
            if (!visited[j] && graph[currentStop][j].stop != -1)
            {
                float newDistance = distances[currentStop] + graph[currentStop][j].weight;
                if (newDistance < distances[j])
                {
                    distances[j] = newDistance;
                    parents[j] = currentStop;
                    buses[j] = graph[currentStop][j].bus;
                }
            }
        }
    }

    if ((int)distances[destination] > 0)
    {
        printf("Distance : %.1f Km\n", distances[destination]);
        printf("Shortest path from STATION ( %s ) to STATION ( %s ) \n", s, d);
        printPath(parents, destination, buses);
        printf("\n");
    }
    else
    {
        printf("---- No Path ----\n");
    }
}

void initialGraph()
{
    for (int i = 0; i < stationLen; i++)
    {
        for (int j = 0; j < stationLen; j++)
        {
            graph[i][j].stop = -1;
            graph[i][j].weight = -1;
            graph[i][j].bus = -1;
        }
    }
    for (int i = 0; i < busLen; i++)
    {

        for (int j = 0; j < stationList[i].stationLen; j++)
        {
            int stationID1 = findStationID(stationList[i].sname[j]);

            for (int k = 0; k < stationList[i].stationLen; k++)
            {
                int stationID2 = findStationID(stationList[i].sname[k]);

                if (adjMatrix[i].adjM[j][k] > 0)
                {
                    addEdge(stationID1, stationID2, adjMatrix[i].adjM[j][k], atoi(adjMatrix[i].bnum));
                }
            }
        }
    }
}

void AllStationForPrint()
{
    Node *ptr = allStation;
    int i = 0;
    while (ptr != NULL)
    {
        printf("  - %-18s  ", ptr->data);
        i++;
        if (i == 2)
        {
            printf("\n");
            i = 0;
        }
        ptr = ptr->next;
    }
    if (i == 1)
        printf("\n");
}

void AllBusForPrint()
{
    for (int i = 0; i < busLen; i++)
    {
        printf(" [ Bus %-3s ] ", stationList[i].bnum);
        printf("Station list : ");
        for (int j = 0; j < stationList[i].stationLen; j++)
        {
            printf("( %s )  ", stationList[i].sname[j]);
            if (j < stationList[i].stationLen - 1)
                printf("➤  ");
        }
        printf("\n");
    }
}

void saveAdjMatrix(int size, float adjacency_matrix[][size], char busNum[], char *newStationList[])
{
    char filename[strlen(busNum) + 4];
    strcpy(filename, busNum);
    strcat(filename, ".csv");
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Error: could not open file %s\n", filename);
        return;
    }

    // Write the header row
    for (int i = 0; i < size + 1; i++)
    {
        if (i == 0)
        {
            fprintf(file, "%s,", busNum);
        }
        else
        {
            fprintf(file, "%s", newStationList[i - 1]);
            if (i != size)
            {
                fprintf(file, ",");
            }
        }
    }
    fprintf(file, "\n");

    // Write the matrix
    for (int i = 0; i < size; i++)
    {
        fprintf(file, "%s,", newStationList[i]);
        for (int j = 0; j < size; j++)
        {
            fprintf(file, "%.1f", adjacency_matrix[i][j]);
            if (j != size - 1)
            {
                fprintf(file, ",");
            }
        }
        fprintf(file, "\n");
    }
    fclose(file);
    printf("--- Completed ---\n");
}

void saveBusList(char *NewBusNum)
{
    FILE *fp;
    fp = fopen(BUSLIST_FILE, "w");
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    for (int i = 0; i < busLen; i++)
    {
        fprintf(fp, "%s\n", stationList[i].bnum);
    }
    fprintf(fp, "%s", NewBusNum);
    fclose(fp);
}

int addBus()
{
    char NewBusNum[MAX];
    printf("Enter Bus number : ");
    scanf("%s", NewBusNum);
    while (getchar() != '\n')
        ;

    int NewStationNum;
    printf("Enter Number of Stations : ");
    scanf("%d", &NewStationNum);
    while (getchar() != '\n')
        ;
    if (NewStationNum < 2)
    {
        printf("Error : Number of stations must be more than 1\n");
        return 0;
    }

    char *newStationList[NewStationNum];
    for (int i = 0; i < NewStationNum; i++)
    {
        char buf[MAX];
        printf("[%d] Enter Station Name : ", i + 1);
        fgets(buf, MAX, stdin);
        newStationList[i] = (char *)malloc(sizeof(buf));
        buf[strcspn(buf, "\r\n")] = 0;
        strcpy(newStationList[i], buf);
    }

    float NewAdjMatrix[NewStationNum][NewStationNum];
    for (int i = 0; i < NewStationNum; i++)
    {
        for (int j = 0; j < NewStationNum; j++)
        {
            if (i == j)
            {
                NewAdjMatrix[i][j] = 0;
                continue;
            }
            printf("Enter distance between [%s] to [%s] (Kilometer) : ", newStationList[i], newStationList[j]);
            scanf("%f", &NewAdjMatrix[i][j]);
        }
    }
    saveAdjMatrix(NewStationNum, NewAdjMatrix, NewBusNum, newStationList);
    saveBusList(NewBusNum);
    getBusStationFromFile();
    createUnionStation();
    return 1;
}

void deleteBus()
{
    char deleteBusNum[MAX];
    printf("Enter Bus Number : ");
    scanf("%s", deleteBusNum);
    while (getchar() != '\n')
        ;

    char filename[strlen(deleteBusNum) + 4];
    strcpy(filename, deleteBusNum);
    strcat(filename, ".csv");
    if ((filename) == 0)
    {
        printf("Bus %s Deleted Successfully\n", deleteBusNum);
    }
    else
    {
        printf("Error: Unable to Delete the Bus %s\n", deleteBusNum);
        return;
    }

    FILE *fp;
    fp = fopen(BUSLIST_FILE, "w");
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        exit(1);
    }
    for (int i = 0; i < busLen; i++)
    {
        if (strcmp(stationList[i].bnum, deleteBusNum) != 0)
        {
            if (i == busLen - 2)
            {
                fprintf(fp, "%s", stationList[i].bnum);
            }
            else
            {
                fprintf(fp, "%s\n", stationList[i].bnum);
            }
        }
    }
    fclose(fp);
    getBusStationFromFile();
    createUnionStation();
}

void printMenu()
{
    printf("🟠%s------------------ Welcome! --------------------%s🟠\n", "\033[38;2;225;128;0m", "\x1B[0m");
    printf("%s|                                                 |%s\n", "\033[38;2;225;128;0m", "\x1B[0m");
    printf("%s|                    %sBUS2GO!%s🚌💨                  %s|%s\n", "\033[38;2;225;153;51m", "\033[4;38;2;225;153;51m", "\x1B[0m", "\033[38;2;225;153;51m", "\x1B[0m");
    printf("%s|        _________________________________        |%s\n", "\033[38;2;225;178;102m", "\x1B[0m");
    printf("%s|       |                                 |       |%s\n", "\033[38;2;225;178;102m", "\x1B[0m");
    printf("%s|       |    [1] Show all station         |       |%s\n", "\033[38;2;225;204;153m", "\x1B[0m");
    printf("%s|       |    [2] Show all bus             |       |%s\n", "\033[38;2;225;204;153m", "\x1B[0m");
    printf("%s|       |    [3] Find the shortest path   |       |%s\n", "\033[38;2;225;204;153m", "\x1B[0m");
    printf("%s|       |    [4] Add Bus to the list      |       |%s\n", "\033[38;2;225;178;102m", "\x1B[0m");
    printf("%s|       |    [5] Delete Bus from the list |       |%s\n", "\033[38;2;225;178;102m", "\x1B[0m");
    printf("%s|       |    [6] Quit                     |       |%s\n", "\033[38;2;225;178;102m", "\x1B[0m");
    printf("%s|       |_________________________________|       |%s\n", "\033[38;2;225;153;51m", "\x1B[0m");
    printf("%s|                                                 |%s\n", "\033[38;2;225;128;0m", "\x1B[0m");
    printf("🟠%s------------------------------------------------%s🟠\n\n", "\033[38;2;225;128;0m", "\x1B[0m");
}
void menuOperation()
{
    printMenu();
    int menu;
    printf("Enter the MENU : ");
    scanf("%d", &menu);
    while (menu != 6)
    {
        if (menu == 1)
        {
            printf("\n🚌💨%s------------------------------------------%s🚌💨 \n", "\033[38;2;225;128;0m", "\x1B[0m");
            printf("           All stations contain of\n");
            printf("   ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  ↓  \n");
            AllStationForPrint();
            printf("🚌💨%s-------------------------------------------%s🚌💨 \n\n", "\033[38;2;225;128;0m", "\x1B[0m");
        }
        else if (menu == 2)
        {
            printf("\n🚌💨%s------------------------------------------%s🚌💨 \n", "\033[38;2;225;128;0m", "\x1B[0m");
            printf("              All bus contain of\n");
            AllBusForPrint();
            printf("🚌💨%s-------------------------------------------%s🚌💨 \n\n", "\033[38;2;225;128;0m", "\x1B[0m");
        }
        else if (menu == 3)
        {
            char source[100];
            char destination[100];
            initialGraph();
            printf("\n🚌💨%s------------------------------------------%s🚌💨 \n", "\033[38;2;225;128;0m", "\x1B[0m");
            printf("              Find the Shortest Path\n\n");
            printf("Initial point : ");
            while (getchar() != '\n')
                ;
            fgets(source, 100, stdin);
            source[strcspn(source, "\r\n")] = 0;
            printf("Destination : ");
            fgets(destination, 100, stdin);
            destination[strcspn(destination, "\r\n")] = 0;
            dijkstra(source, destination);
            printf("🚌💨%s------------------------------------------%s🚌💨 \n\n", "\033[38;2;225;128;0m", "\x1B[0m");
        }
        else if (menu == 4)
        {
            printf("\n🚌💨%s------------------------------------------%s🚌💨 \n", "\033[38;2;225;128;0m", "\x1B[0m");
            printf("               Add Bus to the List\n\n");
            addBus();
            printf("🚌💨%s------------------------------------------%s🚌💨 \n\n", "\033[38;2;225;128;0m", "\x1B[0m");
        }
        else if (menu == 5)
        {
            printf("\n🚌💨%s------------------------------------------%s🚌💨 \n", "\033[38;2;225;128;0m", "\x1B[0m");
            printf("             Delete Bus from the List\n\n");
            deleteBus();
            printf("🚌💨%s------------------------------------------%s🚌💨 \n\n", "\033[38;2;225;128;0m", "\x1B[0m");
        }
        printMenu();
        printf("Enter the MENU : ");
        scanf("%d", &menu);
    }
    printf("\n🚌💨%s-------------------------------------------%s🚌💨 \n", "\033[38;2;225;128;0m", "\x1B[0m");
    printf(" %s|                                             |%s\n", "\033[38;2;225;178;102m", "\x1B[0m");
    printf(" %s|%s               🙇 THANKYOU 🙇                %s|%s\n", "\033[38;2;225;204;153m", "\x1B[0m", "\033[38;2;225;204;153m", "\x1B[0m");
    printf(" %s|                                             |%s\n", "\033[38;2;225;178;102m", "\x1B[0m");
    printf("🚌💨%s-------------------------------------------%s🚌💨 \n\n", "\033[38;2;225;128;0m", "\x1B[0m");
    return;
}

int main()
{
    getBusStationFromFile();
    createUnionStation();
    menuOperation();
    return 0;
}