//
// Created by sunzhijun on 2018/2/17.
//

#ifndef PROJ_ANDROID_STUDIO_ASTAR_H
#define PROJ_ANDROID_STUDIO_ASTAR_H

#include <cstdlib>
#include<vector>
#include <string>
#include <queue>
#include <map>

class AStar{
private:
    struct IntPoint{
        int x;
        int y;
    };
    struct IntEdge{
        IntPoint start;
        IntPoint end;
    };

    static IntPoint sequence_z[2][6];
    //出发点col,row
    static IntPoint source;
    //目的点的col,row
    static IntPoint target_all;
    //结束点col, row
    static IntPoint _target;
//0代表未去过，1代表去过
    static int** _visited;
//A*用优先级队列
    typedef int(*INTPARR)[2];
//A*用优先级队列容器中的比较器，内部重载了（）操作符，此为C++中的函数对象
    struct cmp
    {
        bool operator()(IntEdge& a, IntEdge& b){
            IntPoint& a_end = a.end;
            IntPoint& b_end = b.end;
            int dis_a = _visited[a_end.y][a_end.x] + abs(a_end.x - _target.x) + abs(a_end.y - _target.y);
            int dis_b = _visited[b_end.y][b_end.x] + abs(b_end.x - _target.x) + abs(b_end.y - _target.y);

            return dis_a > dis_b;
        }

    };
//A*用优先级队列
    std::priority_queue<IntEdge,std::vector<IntEdge>,cmp>* _a_star_queue;
    //结果路径记录
    std::map<std::string, IntEdge>* hm_;

    void VisitedNew(int row, int col);
    void VisitedDelete();
    //初始化去过未去过的数组
    void VisitedInit();
    void MapDataNew(int row, int col);
    void MapDataDelete();
};
#endif //PROJ_ANDROID_STUDIO_ASTAR_H
