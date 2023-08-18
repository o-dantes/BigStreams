#include "chrono"
#include <iostream>
#include <vector>
#include <ctime>
#include <string>
#include <thread>
#include "algorithm"
#include "atomic"
//#define REL
#define TEST

class Element
{
public:
    Element();
    Element(time_t time, int value);
    Element(long timestamp);//to handle NULL as passing argument
    Element(int value);
    int SetValue(int value);//undone
    int SetTimeSt(time_t time);//undone
    time_t GetTime() const;//return _timestamp
    int GetValue();
    void Show();//prints "_timestamp value \n"
    Element operator=(const Element& other);//to compare
    bool operator!=(const Element& other);//to compare
private:
    time_t _timestamp; //unix timestamp of creation
    int _value;
};

class Storage
{
public:
    Storage();
    Storage(std::vector<Element>& old, std::vector<Element>& neW);
    ~Storage();// empty
    int Loader(const Element& elem);//adds to storage (for new or for both ifdef REL)
    int Loader(long timestamp);//for use by a Copier
    int Copier();//copies from one vector(storage) to another
    std::vector<Element> GetLogs(time_t start, time_t finish);//gives all elements with timestamps between start and finish
    Element GetLatest();//returns element with the biggest timestamp
    void AddNewEES(int r_from,int r_to);
    void ShowStorage(int choose);
private:
    std::vector<Element> _old;
    std::vector<Element> _new;
};

std::string TimestampToDate(time_t timestamp);
int GivePos(const std::vector<Element>& elements,const Element& elem);
std::atomic <bool> CopyingFinished(false);

int main()
{
    int choose=2;
    int i=0;
    std::vector<Element> Logs;
    std::vector<Element> Old;
    std::vector<Element> New;
    for(i=1;i<11;i++)
    {
        Old.push_back(Element(i));
    }
    Storage newStorage(Old, New);
    
    // Adding elements to _new vector every second
    std::thread addElementsThread([&newStorage]() {
        newStorage.AddNewEES(11,20);
    });
    
    // Copying elements from _old to _new
   // newStorage.Copier(Old, New);
    //std::this_thread::sleep_for(std::chrono::seconds(10));
    
    addElementsThread.join();
    newStorage.Copier();
    newStorage.ShowStorage(choose);
    //newStorage.ShowStorage(choose=1);
    time_t r_from;
    time_t r_to;
    std::cout<<"Enter start "<<std::endl;
    std::cin>>r_from;
    std::cout<<"Enter end "<<std::endl;
    std::cin>>r_to;
    Logs=newStorage.GetLogs(r_from,r_to);
    if(!Logs.empty())
    {
        i=0;
        while(i<Logs.size())
        {
            Logs[i].Show();
            i++;
        }
    }
    return 0;
}

Element::Element()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    _timestamp = std::chrono::system_clock::to_time_t(now);
    _value = 0;
}

Element::Element(long timestamp)
{
    _timestamp=timestamp;
    _value=0;
}

Element::Element(int value)
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    _timestamp = std::chrono::system_clock::to_time_t(now);
    _value=value;
    
}

Element::Element(time_t time, int value)
{
    _timestamp=time;
    _value=value;
}

void Element::Show()
{
    std::cout << _timestamp << " " << _value << std::endl;
}
int Element::GetValue()
{
    return _value;
}
time_t Element::GetTime() const
{
    return this->_timestamp;
}

Element Storage::GetLatest()
{
    if (_new.empty() && _old.empty())
    {
        return Element(NULL);
    }

    Element* maxElem = nullptr;

    for (Element& elem : _old)
    {
        if (!maxElem || elem.GetTime() > maxElem->GetTime())
        {
            maxElem = &elem;
        }
    }

    for (Element& elem : _new)
    {
        if (!maxElem || elem.GetTime() > maxElem->GetTime())
        {
            maxElem = &elem;
        }
    }

    return *maxElem;
}


std::vector<Element> Storage::GetLogs(time_t start, time_t finish)
{
    std::vector<Element> dates;
    for (Element& elem : _new)
    {
        if (elem.GetTime() >= start && elem.GetTime() <= finish)
        {
            dates.push_back(elem);
        }
    }
    if(!CopyingFinished)
    {
        for (Element& elem : _old)
        {
            if (elem.GetTime() >= start && elem.GetTime() <= finish)
            {
                dates.push_back(elem);
            }
        }
    }
    return dates;
}


Storage::Storage(std::vector<Element>& old, std::vector<Element>& neW)
{
    _old = old;
    _new = neW;
}

Storage::~Storage()
{
    //vectors will be automatically deallocated
}
// Solution 5.1 (Add on top)
int Storage::Loader(const Element& elem)
{
#ifndef REL
    _new.insert(_new.begin(), elem);
#endif

#ifdef REL
    _new.insert(_new.begin(),elem);
    _old.push_back(elem);
#endif
    return 0;
}

//Solution 5.1 (for use by a Copier)
int Storage::Loader(long timestamp)
{
    _new.push_back(Element(timestamp));
    return 0;
}
//Solution 5.3
//int Storage::Loader(const Element& elem)
//{
//#ifndef REL
//
//    _new.insert(_new.begin()+GivePos(_new, elem),elem);
//
//#endif
//
//#ifdef REL
//    _new.insert(_new.begin()+GivePos(_new, elem),elem);
//    _old.insert(_old.begin()+GivePos(_old, elem),elem);
//#endif
//    return 0;
//}

int Storage::Copier()
{
    if (!_old.empty() && !_new.empty())
    {
        //reverse for Solution 5.1
        std::reverse(_old.begin(),_old.end());
        for (Element& elem : _old)
        {
            Loader(elem.GetTime()); // passing timestamp
        }
    }
    else
    {
        return 1; // Non-existing vectors
    }
    CopyingFinished.store(true);
    return 0;
     
}

std::string TimestampToDate(time_t timestamp)
{
    std::tm* timeinfo = std::localtime(&timestamp);
    char buffer[11]; // Buffer to hold the formatted date "dd.mm.yyyy"
    
    if (timeinfo != nullptr && std::strftime(buffer, sizeof(buffer), "%d.%m.%Y", timeinfo))
    {
        return std::string(buffer);
    } else
    {
        return "Invalid Timestamp"; // Handle error case
    }
}

Element Element::operator=(const Element& other)
{
    if (this != &other)
    {
        _timestamp = other._timestamp;
        _value = other._value;
    }
    return *this;
}

bool Element::operator!=(const Element& other)
{
    return (_timestamp != other._timestamp) || (_value != other._value);
}

void Storage::AddNewEES(int r_from,int r_to)
{
    int value = r_from;
    while (value <= r_to)
    {
        Loader(Element{ value });

        
        value++;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
void Storage::ShowStorage(int choose)
{
    if(choose!=1 && choose!=2)
    {
        return;
    }
    if(choose==1)
    {
        for (Element& elem : _old)
        {
            elem.Show();
        }
    }
    if(choose==2)
    {
        for (Element& elem : _new)
        {
            elem.Show();
        }
    }
}

//O(N)
//int GivePos(const std::vector<Element>& elements,const Element& elem)
//{
//    for (int i = 0; i < elements.size(); ++i)
//    {
//        if (elements[i].GetTime() <= elem.GetTime() && (i == elements.size() - 1 || elements[i + 1].GetTime() > elem.GetTime()))
//        {
//            return i + 1;
//        }
//    }
//
//    return 0;
//}

//O(LogN)
int GivePos(const std::vector<Element>& elements,const Element& elem)
{
    int low = 0;
    int high = elements.size();

    while (low < high)
    {
        int mid = low + (high - low) / 2;

        if (elements[mid].GetTime() <= elem.GetTime())
        {
            if (mid == elements.size() - 1 || elements[mid + 1].GetTime() > elem.GetTime())
            {
                return mid+1;
            }
            else
            {
                low = mid+1;
            }
        }
        else
        {
            high = mid;
        }
    }

    return 0;
}

