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


time_t SEED; // ���F��replacement�����t��k���ͦP��page�i��ݭn�ק���v(�]�N�Ox�|�@��)

struct page { // �s��page �� dirty bit�B reference bit �B��ARB�t��k�Ϊ�bit stream
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

    void assign_ref_with_random(vector<int>&); // �üƤ��t [1,25] �s�� reference string

    void assign_ref_with_locality(vector<int>&); // �Hfunction locality���Φ����treference string

    void assign_ref_with_continuity(vector<int>&); // �Hfuction locality�åH�s�򪺧Φ����treference

    void FIFO(vector<int>, vector<page>); // �ϥ�FIFO����page replacement

    void ARB(vector<int>, vector<page>); // �ϥ�ARB����page replacement

    void ADB(vector<int>, vector<page>); // �ϥ�ADB����page replacement

    void ESC(vector<int>, vector<page>); // �ϥ�ESC����page replacement

};

void Solution::assign_ref_with_random(vector<int>& ref_string) {
    srand(time(NULL)); // random seed
    int i = 0;
    while(i < REF_SIZE) {
        int x = rand() % 800 + 1; // �üƥX�@�� reference string
        int len = rand() % 25 + 1; //�üƥX�@���H���s�����
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
        int len = (rand() % ((21*(REF_SIZE/FUNCTION_NUM)/20) - (19*(REF_SIZE/FUNCTION_NUM)/20) + 1)) + (19*(REF_SIZE/FUNCTION_NUM)/20); // random�X�C��function�ҭn��reference string ����;random�X[low, up]��������rand() %  (up - low + 1) + low
        //cout << "  len: " <<len <<endl;
        int lower = rand() % 800 + 1; // ����subnet���_�lpage number
        int upper = lower + 800/15 -1; // ����subnet������page number
        if(upper > 800) { // upper�W�L800�Ұ����վ�
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
    int lower = rand() % 800 + 1; // ����subnet���_�lpage number
    int upper = lower + 800/15 -1; // ����subnet������page number
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
        int len = (rand() % ((21*(REF_SIZE/FUNCTION_NUM)/20) - (19*(REF_SIZE/FUNCTION_NUM)/20) + 1)) + (19*(REF_SIZE/FUNCTION_NUM)/20); // random�X�C��function�ҭn��reference string ����;random�X[low, up]��������rand() %  (up - low + 1) + low
        //cout << "  len: " <<len <<endl;
        int lower = rand() % 800 + 1; // ����subnet���_�lpage number
        int upper = lower + 800/7 -1; // ����subnet������page number
        if(upper > 800) { // upper�W�L800�Ұ����վ�
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
    int lower = rand() % 800 + 1; // ����subnet���_�lpage number
    int upper = lower + 800/15 -1; // ����subnet������page number
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

        if(find(frames.begin(), frames.end(), ref_string[i]) != frames.end()) {  // �p�Gframes�̭��w�g�s�b��page�h���L
            continue;
        }

        if(frames.size() < FRAME_SIZE) { // �p�Gframes�٦��Ŷ�
            frames.push_back(ref_string[i]);
        }else { // �p�Gframes�S���Ŷ�
            if(pages[frames.front()].dirty == true) {
                pages[frames.front()].dirty = false;
                disk_write++; // �Y�n�Q�m����page��dirty bit �� 1�A �hdisk writes���ƥ[1
            }
            frames.pop_front();
            frames.push_back(ref_string[i]);
        }

        page_fault++;
    }

    while(!frames.empty()) { //�̫�d�bframes�̪�pages�]�n�ݬݬO�_�ݭn�g�^
        if(pages[frames.front()].dirty == true) disk_write++;
        frames.pop_front();
    }

    cout << "page fault: " << page_fault << endl;
    cout << "interrupts: " << interrupt << endl;
    cout << "disk writes: " << disk_write << endl;
}

void Solution::ARB(vector<int> ref_string, vector<page> pages) { // ����W�OFIFO���Ҽ{�Freference bit
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

        if(find(frames.begin(), frames.end(), ref_string[i]) != frames.end()) {  // �p�Gframes�̭��w�g�s�b��page�h���L
            pages[ref_string[i]].bs[7] = 1; // �`�NŪ���쪺page��U��bit stream�̰���n�]��1
            if((i+1) % period == 0) { // �Cperiod��page reference�ᰵ�@��shifting
                interrupt++;
                for(int j=1; j<=800; j++) {
                    pages[j].bs >>= 1;
                }
            }
            continue;
        }

        if(frames.size() < FRAME_SIZE) { // �p�Gframes�٦��Ŷ�
            frames.push_back(ref_string[i]);
            pages[ref_string[i]].bs[7] = 1; // �`�NŪ���쪺page��U��bit stream�̰���n�]��1
        }else { // �p�Gframes�S���Ŷ�

            int victime_page = frames[0];
        //cout << "----------search start"<<endl;
            for(int i=1; i<frames.size(); i++) { // �z�Lbit stream��Xvictim page
                //cout << frames[i] << ": " << pages[frames[i]].bs.to_ulong()<<  " dirty: "  << pages[frames[i]].dirty<<endl;
                if(pages[frames[i]].bs.to_ulong() < pages[victime_page].bs.to_ulong()) {
                    victime_page = frames[i];
                }
            }
            //cout << "vic " << victime_page << ": " << pages[victime_page].bs.to_ulong()<<endl;
            stack<int> tmp;
            while(1) { // �R��victime page in frames
                if(frames.back() == victime_page) {
                    frames.pop_back();
                    break;
                }

                tmp.push(frames.back());
                frames.pop_back();
            }

            while(!tmp.empty()) { // �Nvictime page���᪺pages��^�hdeque
                frames.push_back(tmp.top());
                tmp.pop();
            }
            if(pages[victime_page].dirty == true) {
                pages[victime_page].dirty = false;
                disk_write++;
            }

            frames.push_back(ref_string[i]);
            pages[ref_string[i]].bs[7] = 1; // �`�NŪ���쪺page��U��bit stream�̰���n�]��1

        }

        page_fault++;
        if((i+1) % period == 0) { // �Cperiod��page reference�ᰵ�@��shifting
            interrupt++;
            for(int j=1; j<=800; j++) {
                pages[j].bs >>= 1;
            }


        }
    }

    while(!frames.empty()) { //�̫�d�bframes�̪�pages�]�n�ݬݬO�_�ݭn�g�^
        if(pages[frames.front()].dirty == true) disk_write++;
        frames.pop_front();
    }

    cout << "page fault: " << page_fault << endl;
    cout << "interrupts: " << interrupt << endl;
    cout << "disk writes: " << disk_write << endl;

}


void Solution::ADB(vector<int> ref_string, vector<page> pages) { // ����W�OFIFO���Ҽ{�Fdirty bit
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

        if(find(frames.begin(), frames.end(), ref_string[i]) != frames.end()) {  // �p�Gframes�̭��w�g�s�b��page�h���L
            if(pages[ref_string[i]].dirty) { // �p�Gdirty bit��1
                pages[ref_string[i]].bs[7] = 1;
            }

            if((i+1) % period == 0) { // �Cperiod��page reference�ᰵ�@��shifting
                interrupt++;
                for(int j=1; j<=800; j++) {
                    pages[j].bs >>= 1;
                }
            }
            continue;
        }

        if(frames.size() < FRAME_SIZE) { // �p�Gframes�٦��Ŷ�
            frames.push_back(ref_string[i]);
            if(pages[ref_string[i]].dirty) {
                pages[ref_string[i]].bs[7] = 1;
            }

        }else { // �p�Gframes�S���Ŷ�

            int victime_page = frames[0];
        //cout << "----------search start"<<endl;
            for(int i=1; i<frames.size(); i++) { // �z�Lbit stream��Xvictim page
                //cout << frames[i] << ": " << pages[frames[i]].bs.to_ulong()<<  " dirty: "  << pages[frames[i]].dirty<<endl;
                if(pages[frames[i]].bs.to_ulong() < pages[victime_page].bs.to_ulong()) {
                    victime_page = frames[i];
                }
            }
            //cout << "vic " << victime_page << ": " << pages[victime_page].bs.to_ulong()<<endl;
            stack<int> tmp;
            while(1) { // �R��victime page in frames
                if(frames.back() == victime_page) {
                    frames.pop_back();
                    break;
                }

                tmp.push(frames.back());
                frames.pop_back();
            }

            while(!tmp.empty()) { // �Nvictime page���᪺pages��^�hdeque
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
        if((i+1) % period == 0) { // �Cperiod��page reference�ᰵ�@��shifting
            interrupt++;
            for(int j=1; j<=800; j++) {
                pages[j].bs >>= 1;
            }


        }
    }

    while(!frames.empty()) { //�̫�d�bframes�̪�pages�]�n�ݬݬO�_�ݭn�g�^
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


        if(find(frames.begin(), frames.end(), ref_string[i]) != frames.end()) {  // �p�Gframes�̭��w�g�s�b��page�h���L
            //cout << "already" <<endl;
            continue;
        }

        if(frames.size() < FRAME_SIZE) { // �p�Gframes�٦��Ŷ�
            //cout <<"\n";
            frames.push_back(ref_string[i]);
        }else { // �p�Gframes�S���Ŷ�
            interrupt++;
            int flag = 0;
            for(int j=0; j<FRAME_SIZE; j++, index=(index+1)%FRAME_SIZE) { // �Ĥ@������reference bit:dirty bit��00��victim page
                //cout << "\n"<<"\t"<<"index: " <<index<< " frames[index]:" <<frames[index]<< " refer: " << pages[frames[index]].refer<<" dirty: "<<pages[frames[index]].dirty <<endl;
                if(pages[frames[index]].refer == false && pages[frames[index]].dirty == false) { // 00
                    //cout << "1 " << frames[index]  << " "<<pages[ref_string[i]].refer<<endl;
                    frames[index] = ref_string[i];
                    flag = 1;
                    index = (index+1)%FRAME_SIZE;
                    break;
                }
            }
            if(flag == 0) { // �Ĥ@���S����Xvictim page
                for(int j=0; j<FRAME_SIZE; j++, index=(index+1)%FRAME_SIZE) { // �ĤG������reference bit:dirty bit��01��victim page
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

            if(flag == 0) { // �Nframe��pages��reference bit�令0
                for(int j=0; j<frames.size(); j++) {
                    pages[frames[j]].refer = false;
                }

            }

            if(flag == 0) { // �ĤG���S����Xvictim page
                for(int j=0; j<FRAME_SIZE; j++, index=(index+1)%FRAME_SIZE) { // �ĤT������reference bit:dirty bit��00��victim page
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
            if(flag == 0) { // �ĤT���S����Xvictim page
                for(int j=0; j<FRAME_SIZE; j++, index=(index+1)%FRAME_SIZE) { // �ĥ|������reference bit:dirty bit��01��victim page
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

    while(!frames.empty()) { //�̫�d�bframes�̪�pages�]�n�ݬݬO�_�ݭn�g�^
        if(pages[frames.front()].dirty == true) disk_write++;
        frames.pop_front();
    }

    cout << "page fault: " << page_fault << endl;
    cout << "interrupts: " << interrupt << endl;
    cout << "disk writes: " << disk_write << endl;

}

int main()
{
    vector<int> ref_string(REF_SIZE); // �s��reference string���ǡA�ñNref_string�j�p�]��REF_SIZE
    vector<page> pages(801); // �s��1~800��pages�A�Npages�j�p�]��801(���F���index)


    Solution sol(pages);
    sol.assign_ref_with_random(ref_string);
    sol.assign_ref_with_locality(ref_string);
    sol.assign_ref_with_continuity(ref_string);
    SEED = time(NULL); // �Nrandom seed�ƥ��s�_�ӡA�H�K�|�Ӻt��k�b����page modify�����v�O�ۦP��

    for(int i=1; i<=5; i++) { // �̷�frane size 20 40 60 80 100���ǰ���
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
