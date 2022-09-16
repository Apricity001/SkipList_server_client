#ifndef MINIREDIS_SKIPLIST_H
#define MINIREDIS_SKIPLIST_H
//#define RANDOM


#include<fstream>
#include <mutex>
#include<string>
#include "Node.h"
//#include "Random.h"
using namespace std;

#define STORE_FILE "D:\\software\\C++Project\\MiniRedis\\Disk\\dumpFile.txt"
string delimiter =":";

mutex mtx;


// skiplist类

template <typename K, typename V>
class Skiplist
{
public:
    Skiplist();
    Skiplist(int);
    ~Skiplist();
    int get_random_level();
    Node<K, V> *create_node(K, V, int);
    void display_list();
    int insert_element(K, V);
    //int change_element(K,V);
    bool search_element(K,V &);
    bool delete_element(K);
    
    int size();
    
    void dump_file();
    void load_file();

private:
    void get_key_value_from_string(const string &str, string *key, string *value);
    bool is_valid_string(const string &str);

private:
    //跳表的最大层数
    int _max_level;

    //当前所在的层数
    int _skip_list_level;

    //跳表头节点指针
    Node<K, V> *_header;

    //当前元素个数
    int _element_count;

    ofstream _file_writer;
    ifstream _file_reader;
#ifdef RANDOM
    Random rnd;
#endif
};

//创造节点
template <typename K, typename V>
Node<K, V> *Skiplist<K, V>::create_node(const K k, const V v, int level)
{
    Node<K, V> *n = new Node<K, V>(k, v, level);
    return n;
}

template <typename K, typename V>
Skiplist<K, V>::Skiplist()
{
    this->_max_level = 32;
    this->_skip_list_level = 0;
    this->_element_count = 0;
#ifdef RANDOM
    this->rnd = 0x12345678;
#endif

    //创建头节点 并将K与V初始化为NULL
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
}


//建造跳表
template <typename K, typename V>
Skiplist<K, V>::Skiplist(int max_level)
{
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;
#ifdef RANDOM
    this->rnd=0x12345678;
#endif
    
    //创建头节点 并将K与V初始化为NULL
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
}

//析构跳表
template <typename K, typename V>
Skiplist<K, V>::~Skiplist()
{
    if(_file_reader.is_open()){
        _file_reader.close();
    }
    if(_file_writer.is_open()){
        _file_writer.close();
    }
    delete _header;
}

// Insert given key and value in skip list  返回值为int 为后续拓展留下空间
// return 0 means element exists  insert failed
// return 1 means insert successfully

/*
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+

*/

template <typename K, typename V>
int Skiplist<K, V>::insert_element(K key, V value)
{
    mtx.lock();
    Node<K, V> *current = this->_header;

    //创建数组update并初始化该数组。
    // update 是一个将节点node。。。
    Node<K, V>* update = new Node<K, V>[_max_level + 1];
    //Node<K, V> *update[_max_level + 1]=new ;
    //memset(update, 0, sizeof(Node<K, V> *) * (_skip_list_level + 1));

    //从跳表的最左上角节点开始查找
    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->forward[i] != nullptr && current->forward[i]->getKey() < key)
        {
            current = current->forward[i];
        }
        update[i] = *current;
    }

    //到达第0层，并且当前的forward 指针指向第一个大于待插入节点的节点。
    current = current->forward[0];

    //如果当前节点的key值和待插入节点key相等，则说明待插入节点值存在。修改该节点的值。
    if (current != nullptr && current->getKey() == key)
    {
        cout << "key: " << key << ", exists. Change it" << endl;
        current->setValue(value);
        mtx.unlock();
        return 0;
    }

    //如果current节点为null 这就意味着要将该元素插入最后一个节点。
    //如果current的key值和待插入的key不等，代表我们应该在update[0]和current之间插入该节点。
    if (current == nullptr || current->getKey() != key)
    {
        //为该节点计算出一个随机的层次
        int random_level = get_random_level();

        //
        if (random_level > _skip_list_level)
        {
            for (int i = _skip_list_level + 1; i < random_level + 1; i++)
            {
                update[i] =* _header;
            }
            _skip_list_level = random_level;
        }

        //使用random level生成新节点
        Node<K, V> *inserted_node = create_node(key, value, random_level);

        //插入节点
        for (int i = 0; i <= random_level; i++)
        {
            inserted_node->forward[i] = update[i].forward[i];
            update[i].forward[i] = inserted_node;
        }
       
        // cout<<"Successfully inserted key: "<<key<<", value: "<<value<<endl;
        _element_count++;
    }
    //delete[]update;
    mtx.unlock();
    return 1;
}

// Search for element in skip list
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
template <typename K, typename V>
bool Skiplist<K, V>::search_element(K key,V &value)
{

    //cout << "search_element-------------" << endl;
    Node<K, V> *current = _header;

    //从跳表的最左上角节点开始寻找
    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->forward[i] && current->forward[i]->getKey() < key)
        {
            current = current->forward[i];
        }
    }

    //到达第0层
    current = current->forward[0];

    if (current->getKey() == key)
    {
        value=current->getValue();
        //cout << "Found key: " << key << " value: " << current->getValue() << endl;
        return true;
    }
    //cout << "No Found Key: " << key << endl;
    return false;
}

template <typename K, typename V>
bool Skiplist<K, V>::delete_element(K key)
{
    mtx.lock();
    //Node<K, V> *update [_max_level+ 1];
    Node<K, V> *update = new Node<K,V>[_max_level + 1];
    //memset(update, 0 , sizeof(Node<K, V>*) * (_skip_list_level + 1));

    Node<K, V> *current = _header;

    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->forward[i] != nullptr && current->forward[i]->getKey() < key)
        {
            current = current->forward[i];
        }
        update[i] = *current;
    }

    current = current->forward[0];

    if (current != nullptr && current->getKey() == key)
    {
        for (int i = 0; i <=_skip_list_level; i++)
        {
            if (update[i].forward[i] != current)
            {
                break;
            }
            update[i].forward[i] = current->forward[i];
        }
        delete current;
        //delete []update;
        cout << "Successfully deleted key: " << key << endl;
        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == nullptr)
        {
            _skip_list_level--;
        }
        _element_count--;
        mtx.unlock();
        return 1;
    }
    else
    {
        mtx.unlock();
        return 0;
    }
}

template <typename K, typename V>
void Skiplist<K, V>::display_list()
{
    cout<<"SkipList:"<<endl;

    Node<K,V> *current;

    for(int i=_skip_list_level;i>=0;i--){
        current=_header->forward[i];
        cout<<"Level "<<i<<':'<<endl;
        while(current!=nullptr){
            cout<<' '<<current->getKey()<<':'<<current->getValue();
            current=current->forward[i];
        }
        cout<<endl;
    }
    return;
}

#ifndef RANDOM
template<typename K, typename V>
int Skiplist<K, V>::get_random_level(){

    int k = 0;
    while (rand() % 2) {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
};
#endif

#ifdef RANDOM
template<typename K, typename V>
int Skiplist<K, V>::get_random_level() {
    int level = static_cast<int>(rnd.Uniform(_max_level));
    if (level == 0) {
        level = 1;
    }
    return level;
}
#endif

template<typename K,typename V>
int Skiplist<K,V>::size(){
    return this->_element_count;
}

template<typename K,typename V>
bool Skiplist<K,V>::is_valid_string(const string& str){
    if(str.empty()){
        return false;
    }
    if(str.find(delimiter)==string::npos){
       return false;
    }
    return true;
}

template<typename K,typename V>
void Skiplist<K,V>::get_key_value_from_string(const string& str,string *key,string *value){
    if(!is_valid_string(str)){
        return;
    }
    *key=str.substr(0,str.find(delimiter));
    *value=str.substr(str.find(delimiter)+1,str.length());
}

template<typename K,typename V>
void Skiplist<K,V>::dump_file(){
    cout<<"dump_file-------------"<<endl;
    _file_writer.open(STORE_FILE);
    Node<K,V> *node=this->_header->forward[0];

    while(node !=NULL){
        _file_writer<<node->getKey()<<":"<<node->getValue()<<"\n";
        cout<<node->getKey()<<":"<<node->getValue()<<";\n";
        node=node->forward[0];
    }
    _file_writer.flush();
    _file_writer.close();
    return;
}


template<typename K,typename V>
void Skiplist<K,V>::load_file(){
    cout<<"load_file-------------"<<endl;
    _file_reader.open(STORE_FILE);
    string line;
    string *key=new string();
    string *value=new string();
    while(getline(_file_reader,line)){
        get_key_value_from_string(line,key,value);
        if(key->empty()||value->empty()){
            continue;
        }
        insert_element(*key,*value);
        cout<<"key: "<<*key<<" value: "<<*value<<endl;
    }

    _file_reader.close();
}



#endif