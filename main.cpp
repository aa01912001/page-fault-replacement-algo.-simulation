#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <bitset>
#include <queue>
#include <algorithm>
#include <stack>



#define REF_SIZE 200000

#define FUNCTION_NUM 100

#define WRITE_PROBA_MODE 100
#define WRITE_PROBA 10


#define PERIOD 20

int FRAME_SIZE = 0;

using namespace std;


time_t SEED; // 為了讓replacement中的演算法產生同樣page可能需要修改機率(也就是x會一樣)

struct page { // 存放page 的 dirty bit、 reference bit 、給ARB演算法用的bit stream
    int page_num;
    bool dirty;
    bool refer;
    bitset<8> bs;
};


class Solution {

public:

    Solution(vector<page>& pages) {

        for(int i=1; i<=800; i++) {
            pages[i].page_num = i;
            pages[i].dirty = false;
            pages[i].refer = false;
            //pages[i].bs[7] = 1;

        }


        //cout << pages[800].bs.to_ulong() <<endl;
        /*pages[100].bs[5] = 1;
        pages[100].bs >>= 1;
        pages[101].bs[7] = 1;
        pages[101].bs >>= 1;
        cout << pages[101].bs.to_ulong() <<endl;*/
    }

    void assign_ref_with_random(vector<int>&); // 亂數分配 [1,25] 連續的 reference string

    void assign_ref_with_locality(vector<int>&); // 以function locality的形式分配reference string

    void assign_ref_with_continuity(vector<int>&); // 以fuction locality並以連續的形式分配reference

    void FIFO(vector<int>, vector<page>); // 使用FIFO執行page replacement

    void ARB(vector<int>, vector<page>); // 使用ARB執行page replacement

    void ADB(vector<int>, vector<page>); // 使用ADB執行page replacement

    void ESC(vector<int>, vector<page>); // 使用ESC執行page replacement

};

void Solution::assign_ref_with_random(vector<int>& ref_string) {
    srand(time(NULL)); // random seed
    int i = 0;
    while(i < REF_SIZE) {
        int x = rand() % 800 + 1; // 亂數出一個 reference string
        int len = rand() % 25 + 1; //亂數出一個隨機連續長度
        //cout << "len: " << len <<endl;
        for(int j=0; j<len; j++) {
            if(x+j > 800) break;
            ref_string[i] = (x + j) % 800  == 0 ? 800 : (x + j) % 800;
            //cout << ref_string[i] << endl;
            if(++i == REF_SIZE) break;
        }
    }
}

void Solution::assign_ref_with_locality(vector<int>& ref_string) {
    srand(time(NULL)); // random seed
    int i;
    int index = 0;
    int cnt;
    for(i=1; i<FUNCTION_NUM; i++) {
        //cout << "functopm num: " << i;
        int len = (rand() % ((21*(REF_SIZE/FUNCTION_NUM)/20) - (19*(REF_SIZE/FUNCTION_NUM)/20) + 1)) + (19*(REF_SIZE/FUNCTION_NUM)/20); // random出每個function所要的reference string 長度;random出[low, up]的公式為rand() %  (up - low + 1) + low
        //cout << "  len: " <<len <<endl;
        int lower = rand() % 800 + 1; // 產生subnet的起始page number
        int upper = lower + 800/15 -1; // 產生subnet的末端page number
        if(upper > 800) { // upper超過800所做的調整
            lower -= upper - 800;
            upper = 800;
        }
        cnt = 0;
        while(cnt < len) {
            ref_string[index] = rand() % (upper - lower + 1) + lower;
            //cout << ref_string[index] << endl;
            index++;
            cnt++;
        }

    }

    //cout << "functopm num: " << FUNCTION_NUM << endl;
    int lower = rand() % 800 + 1; // 產生subnet的起始page number
    int upper = lower + 800/15 -1; // 產生subnet的末端page number
    while(index < REF_SIZE) {
        ref_string[index] = rand() % (upper - lower + 1) + lower;
        //cout << ref_string[index] << endl;
        index++;
    }

}

void Solution::assign_ref_with_continuity(vector<int>& ref_string) {
    srand(time(NULL)); // random seed
    int i;
    int index = 0;
    int cnt;
    for(i=1; i<FUNCTION_NUM; i++) {
        //cout << "functopm num: " << i;
        int len = (rand() % ((21*(REF_SIZE/FUNCTION_NUM)/20) - (19*(REF_SIZE/FUNCTION_NUM)/20) + 1)) + (19*(REF_SIZE/FUNCTION_NUM)/20); // random出每個function所要的reference string 長度;random出[low, up]的公式為rand() %  (up - low + 1) + low
        //cout << "  len: " <<len <<endl;
        int lower = rand() % 800 + 1; // 產生subnet的起始page number
        int upper = lower + 800/7 -1; // 產生subnet的末端page number
        if(upper > 800) { // upper超過800所做的調整
            lower -= upper - 800;
            upper = 800;
        }
        cnt = 0;
        while(cnt < len) {
            ref_string[index] = lower + (cnt % (upper-lower+1));
            //cout << ref_string[index] << endl;
            index++;
            cnt++;
        }

    }

    //cout << "functopm num: " << FUNCTION_NUM << endl;
    int lower = rand() % 800 + 1; // 產生subnet的起始page number
    int upper = lower + 800/15 -1; // 產生subnet的末端page number
    while(index < REF_SIZE) {
        ref_string[index] = lower + (cnt % (upper-lower+1));
        //cout << ref_string[index] << endl;
        index++;
    }

}

void Solution::FIFO(vector<int> ref_string, vector<page> pages) {
    srand(SEED); // random seed
    deque<int> frames;
    int interrupt = 0;
    int disk_write = 0;
    int page_fault = 0;

    for(int i=0; i<ref_string.size(); i++) {
        int x = (rand() % WRITE_PROBA_MODE) + 1;
        if(x <= WRITE_PROBA) {
            pages[ref_string[i]].dirty = true;
        }

        if(find(frames.begin(), frames.end(), ref_string[i]) != frames.end()) {  // 如果frames裡面已經存在該page則跳過
            continue;
        }

        if(frames.size() < FRAME_SIZE) { // 如果frames還有空間
            frames.push_back(ref_string[i]);
        }else { // 如果frames沒有空間
            if(pages[frames.front()].dirty == true) {
                pages[frames.front()].dirty = false;
                disk_write++; // 若要被置換的page的dirty bit 為 1， 則disk writes次數加1
            }
            frames.pop_front();
            frames.push_back(ref_string[i]);
        }

        page_fault++;
    }

    while(!frames.empty()) { //最後留在frames裡的pages也要看看是否需要寫回
        if(pages[frames.front()].dirty == true) disk_write++;
        frames.pop_front();
    }

    cout << "page fault: " << page_fault << endl;
    cout << "interrupts: " << interrupt << endl;
    cout << "disk writes: " << disk_write << endl;
}

void Solution::ARB(vector<int> ref_string, vector<page> pages) { // 本質上是FIFO但考慮了reference bit
    srand(SEED); // random seed
    int period = PERIOD;
    deque<int> frames;
    int interrupt = 0;
    int disk_write = 0;
    int page_fault = 0;
    for(int i=0; i<ref_string.size(); i++) {
        //cout << ref_string[i] << "   "<<endl;
        int x = (rand() % WRITE_PROBA_MODE) + 1;
        if(x <= WRITE_PROBA) {
            pages[ref_string[i]].dirty = true;
        }

        if(find(frames.begin(), frames.end(), ref_string[i]) != frames.end()) {  // 如果frames裡面已經存在該page則跳過
            pages[ref_string[i]].bs[7] = 1; // 注意讀取到的page當下之bit stream最高位要設為1
            if((i+1) % period == 0) { // 每period個page reference後做一次shifting
                interrupt++;
                for(int j=1; j<=800; j++) {
                    pages[j].bs >>= 1;
                }
            }
            continue;
        }

        if(frames.size() < FRAME_SIZE) { // 如果frames還有空間
            frames.push_back(ref_string[i]);
            pages[ref_string[i]].bs[7] = 1; // 注意讀取到的page當下之bit stream最高位要設為1
        }else { // 如果frames沒有空間

            int victime_page = frames[0];
        //cout << "----------search start"<<endl;
            for(int i=1; i<frames.size(); i++) { // 透過bit stream找出victim page
                //cout << frames[i] << ": " << pages[frames[i]].bs.to_ulong()<<  " dirty: "  << pages[frames[i]].dirty<<endl;
                if(pages[frames[i]].bs.to_ulong() < pages[victime_page].bs.to_ulong()) {
                    victime_page = frames[i];
                }
            }
            //cout << "vic " << victime_page << ": " << pages[victime_page].bs.to_ulong()<<endl;
            stack<int> tmp;
            while(1) { // 刪掉victime page in frames
                if(frames.back() == victime_page) {
                    frames.pop_back();
                    break;
                }

                tmp.push(frames.back());
                frames.pop_back();
            }

            while(!tmp.empty()) { // 將victime page之後的pages放回去deque
                frames.push_back(tmp.top());
                tmp.pop();
            }
            if(pages[victime_page].dirty == true) {
                pages[victime_page].dirty = false;
                disk_write++;
            }

            frames.push_back(ref_string[i]);
            pages[ref_string[i]].bs[7] = 1; // 注意讀取到的page當下之bit stream最高位要設為1

        }

        page_fault++;
        if((i+1) % period == 0) { // 每period個page reference後做一次shifting
            interrupt++;
            for(int j=1; j<=800; j++) {
                pages[j].bs >>= 1;
            }


        }
    }

    while(!frames.empty()) { //最後留在frames裡的pages也要看看是否需要寫回
        if(pages[frames.front()].dirty == true) disk_write++;
        frames.pop_front();
    }

    cout << "page fault: " << page_fault << endl;
    cout << "interrupts: " << interrupt << endl;
    cout << "disk writes: " << disk_write << endl;

}


void Solution::ADB(vector<int> ref_string, vector<page> pages) { // 本質上是FIFO但考慮了dirty bit
    srand(SEED); // random seed
    int period = PERIOD;
    deque<int> frames;
    int interrupt = 0;
    int disk_write = 0;
    int page_fault = 0;
    for(int i=0; i<ref_string.size(); i++) {
        //cout << ref_string[i] << "   "<<endl;
        int x = (rand() % WRITE_PROBA_MODE) + 1;
        if(x <= WRITE_PROBA) {
            pages[ref_string[i]].dirty = true;
        }

        if(find(frames.begin(), frames.end(), ref_string[i]) != frames.end()) {  // 如果frames裡面已經存在該page則跳過
            if(pages[ref_string[i]].dirty) { // 如果dirty bit為1
                pages[ref_string[i]].bs[7] = 1;
            }

            if((i+1) % period == 0) { // 每period個page reference後做一次shifting
                interrupt++;
                for(int j=1; j<=800; j++) {
                    pages[j].bs >>= 1;
                }
            }
            continue;
        }

        if(frames.size() < FRAME_SIZE) { // 如果frames還有空間
            frames.push_back(ref_string[i]);
            if(pages[ref_string[i]].dirty) {
                pages[ref_string[i]].bs[7] = 1;
            }

        }else { // 如果frames沒有空間

            int victime_page = frames[0];
        //cout << "----------search start"<<endl;
            for(int i=1; i<frames.size(); i++) { // 透過bit stream找出victim page
                //cout << frames[i] << ": " << pages[frames[i]].bs.to_ulong()<<  " dirty: "  << pages[frames[i]].dirty<<endl;
                if(pages[frames[i]].bs.to_ulong() < pages[victime_page].bs.to_ulong()) {
                    victime_page = frames[i];
                }
            }
            //cout << "vic " << victime_page << ": " << pages[victime_page].bs.to_ulong()<<endl;
            stack<int> tmp;
            while(1) { // 刪掉victime page in frames
                if(frames.back() == victime_page) {
                    frames.pop_back();
                    break;
                }

                tmp.push(frames.back());
                frames.pop_back();
            }

            while(!tmp.empty()) { // 將victime page之後的pages放回去deque
                frames.push_back(tmp.top());
                tmp.pop();
            }
            if(pages[victime_page].dirty == true) {
                pages[victime_page].dirty = false;
                disk_write++;
            }

            frames.push_back(ref_string[i]);
            if(pages[ref_string[i]].dirty) {
                pages[ref_string[i]].bs[7] = 1;
            }


        }

        page_fault++;
        if((i+1) % period == 0) { // 每period個page reference後做一次shifting
            interrupt++;
            for(int j=1; j<=800; j++) {
                pages[j].bs >>= 1;
            }


        }
    }

    while(!frames.empty()) { //最後留在frames裡的pages也要看看是否需要寫回
        if(pages[frames.front()].dirty == true) disk_write++;
        frames.pop_front();
    }

    cout << "page fault: " << page_fault << endl;
    cout << "interrupts: " << interrupt << endl;
    cout << "disk writes: " << disk_write << endl;

}

void Solution::ESC(vector<int> ref_string, vector<page> pages) {
    srand(SEED); // random seed
    deque<int> frames;
    int interrupt = 0;
    int disk_write = 0;
    int page_fault = 0;
    int index = 0;
    int ok = 0;
    for(int i=0; i<ref_string.size(); i++) {
        //cout << ref_string[i] << " : ";
        int x = (rand() % WRITE_PROBA_MODE) + 1;
        if(x <= WRITE_PROBA) {
            pages[ref_string[i]].dirty = true;
        }
        pages[ref_string[i]].refer = true;


        if(find(frames.begin(), frames.end(), ref_string[i]) != frames.end()) {  // 如果frames裡面已經存在該page則跳過
            //cout << "already" <<endl;
            continue;
        }

        if(frames.size() < FRAME_SIZE) { // 如果frames還有空間
            //cout <<"\n";
            frames.push_back(ref_string[i]);
        }else { // 如果frames沒有空間
            interrupt++;
            int flag = 0;
            for(int j=0; j<FRAME_SIZE; j++, index=(index+1)%FRAME_SIZE) { // 第一輪先找reference bit:dirty bit為00的victim page
                //cout << "\n"<<"\t"<<"index: " <<index<< " frames[index]:" <<frames[index]<< " refer: " << pages[frames[index]].refer<<" dirty: "<<pages[frames[index]].dirty <<endl;
                if(pages[frames[index]].refer == false && pages[frames[index]].dirty == false) { // 00
                    //cout << "1 " << frames[index]  << " "<<pages[ref_string[i]].refer<<endl;
                    frames[index] = ref_string[i];
                    flag = 1;
                    index = (index+1)%FRAME_SIZE;
                    break;
                }
            }
            if(flag == 0) { // 第一輪沒有找出victim page
                for(int j=0; j<FRAME_SIZE; j++, index=(index+1)%FRAME_SIZE) { // 第二輪先找reference bit:dirty bit為01的victim page
                    //cout << "\n"<<"\t"<<"index: " <<index<< " frames[index]:" <<frames[index]<< " refer: " << pages[frames[index]].refer<<" dirty: "<<pages[frames[index]].dirty <<endl;
                    if(pages[frames[index]].refer == false && pages[frames[index]].dirty == true) { // 01
                        //cout << "2 " << frames[index]  << " "<<pages[ref_string[i]].refer<<endl;
                        pages[frames[index]].dirty = false;
                        disk_write++;
                        frames[index] = ref_string[i];
                        index = (index+1)%FRAME_SIZE;
                        flag = 1;
                        break;
                    }
                }
            }

            if(flag == 0) { // 將frame中pages的reference bit改成0
                for(int j=0; j<frames.size(); j++) {
                    pages[frames[j]].refer = false;
                }

            }

            if(flag == 0) { // 第二輪沒有找出victim page
                for(int j=0; j<FRAME_SIZE; j++, index=(index+1)%FRAME_SIZE) { // 第三輪先找reference bit:dirty bit為00的victim page
                    //cout << "\n"<<"\t"<<"index: " <<index<< " frames[index]:" <<frames[index]<< " refer: " << pages[frames[index]].refer<<" dirty: "<<pages[frames[index]].dirty <<endl;
                    if(pages[frames[index]].refer == false && pages[frames[index]].dirty == false) { // 00
                        //cout << "3 " << frames[index]  << " "<< pages[ref_string[i]].refer <<endl;
                        frames[index] = ref_string[i];
                        flag = 1;
                        index = (index+1)%FRAME_SIZE;
                        break;
                    }
                }
            }
            if(flag == 0) { // 第三輪沒有找出victim page
                for(int j=0; j<FRAME_SIZE; j++, index=(index+1)%FRAME_SIZE) { // 第四輪先找reference bit:dirty bit為01的victim page
                    if(pages[frames[index]].refer == false && pages[frames[index]].dirty == true) { // 01
                        //cout << "4 " << frames[index]  << " "<<pages[ref_string[i]].refer<<endl;
                        pages[frames[index]].dirty = false;
                        disk_write++;
                        frames[index] = ref_string[i];
                        index = (index+1)%FRAME_SIZE;
                        flag = 1;
                        break;
                    }
                }
            }
        }

        page_fault++;
    }

    while(!frames.empty()) { //最後留在frames裡的pages也要看看是否需要寫回
        if(pages[frames.front()].dirty == true) disk_write++;
        frames.pop_front();
    }

    cout << "page fault: " << page_fault << endl;
    cout << "interrupts: " << interrupt << endl;
    cout << "disk writes: " << disk_write << endl;

}

int main()
{
    vector<int> ref_string(REF_SIZE); // 存放reference string順序，並將ref_string大小設為REF_SIZE
    vector<page> pages(801); // 存放1~800的pages，將pages大小設為801(為了對齊index)


    Solution sol(pages);
    sol.assign_ref_with_random(ref_string);
    sol.assign_ref_with_locality(ref_string);
    sol.assign_ref_with_continuity(ref_string);
    SEED = time(NULL); // 將random seed事先存起來，以便四個演算法在產生page modify的機率是相同的

    for(int i=1; i<=5; i++) { // 依照frane size 20 40 60 80 100順序執行
        cout << "frame size: "<< i*20 <<endl;
        FRAME_SIZE = i * 20;

        sol.assign_ref_with_random(ref_string);
        cout << "random reference string" << endl;
        cout << "\n\n\n";
        cout << "FIFO: " << endl;
        sol.FIFO(ref_string, pages);
        cout << "\n\n\n";
        cout << "ARB: " << endl;
        sol.ARB(ref_string, pages);
        cout << "\n\n\n";
        cout << "ADB: " << endl;
        sol.ADB(ref_string, pages);
        cout << "\n\n\n";
        cout << "ESC: " << endl;
        sol.ESC(ref_string, pages);
        cout << "\n\n\n";

        sol.assign_ref_with_locality(ref_string);
        cout << "locality reference string" << endl;
        cout << "\n\n\n";
        cout << "FIFO: " << endl;
        sol.FIFO(ref_string, pages);
        cout << "\n\n\n";
        cout << "ARB: " << endl;
        sol.ARB(ref_string, pages);
        cout << "\n\n\n";
        cout << "ADB: " << endl;
        sol.ADB(ref_string, pages);
        cout << "\n\n\n";
        cout << "ESC: " << endl;
        sol.ESC(ref_string, pages);
        cout << "\n\n\n";

        sol.assign_ref_with_continuity(ref_string);
        cout << "my reference string" << endl;
        cout << "\n\n\n";
        cout << "FIFO: " << endl;
        sol.FIFO(ref_string, pages);
        cout << "\n\n\n";
        cout << "ARB: " << endl;
        sol.ARB(ref_string, pages);
        cout << "\n\n\n";
        cout << "ADB: " << endl;
        sol.ADB(ref_string, pages);
        cout << "\n\n\n";
        cout << "ESC: " << endl;
        sol.ESC(ref_string, pages);
        cout << "\n\n\n";
    }


    return 0;
}
