#include "header.hpp"

class City {
private:

int population;
string name;
double latitude;
double longitude;
string state_id; 
string city;

public:
// defeault constructor 
    City() : population(0), name(""), latitude(0.0), longitude(0.0), state_id(""), city("") {}

// constructor
    City(int pop, const string& n, double lat, double lon, const string& state, const string& c);
    int getPopulation() const;
    string getName() const;
    double getLatitude() const;
    double getLongitude() const;
    string getStateId() const;
    string getCity() const;

// getters
    int getPopulation() const;
    string getName() const;
    double getLatitude() const;
    double getLongitude() const;
    string getStateId() const;
    string getCity() const; 

    
// setters
    void setPopulation(int pop);
    void setName(const string& n);
    void setLatitude(double lat);
    void setLongitude(double lon);
    void setStateId(const string& state);
    void setCity(const string& c);

};