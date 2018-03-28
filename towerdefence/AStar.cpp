//
// Created by sunzhijun on 2018/2/17.
//
#include "AStar.h"


void GameLayer::MapDataDelete() {

    for(int i=0;i<row_;i++)
    {
        delete []map_data_[i];
    }
    delete []map_data_;

}

void GameLayer::MapDataNew(int row, int col) {

    map_data_ = new int*[row];
    for(int i = 0; i<row; i++)
    {
        map_data_[i] = new int[col];
    }

}

void GameLayer::VisitedNew(int row, int col) {

    _visited = new int*[row];
    for(int i = 0; i<row; i++)
    {
        _visited[i] = new int[col];
    }

}

void GameLayer::VisitedDelete() {

    if(_visited != NULL)
    {
        for(int i=0;i<row_;i++)
        {
            delete []_visited[i];
        }
        delete [] _visited;
        _visited = NULL;
    }

}

//初始化去过未去过的数组
void GameLayer::VisitedInit()
{
    for(int i=0;i<row_;i++)
    {
        for(int j=0;j<col_;j++)
        {
            _visited[i][j] = 0;
        }
    }
}
