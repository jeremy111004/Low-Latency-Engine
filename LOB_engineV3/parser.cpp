#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <charconv>
#include <filesystem>
#include "/home/jeremy1110/LOB_Engine/LOB_engineV3/Order2.hpp"
#include "parser.hpp"
bool isOrderCorrect(std::string_view s){
    int ctr = 0;
    for(auto c : s){
        if(c == ','){ctr++;}
    }
    return ctr == 3;
}
std::vector<InboundJob> fileReaderParser(const char* filename) noexcept {
	int addedorders = 0;
	std::error_code ec;
	std::filesystem::path filePath
        = filename;
	
	std::ifstream f("/home/jeremy1110/LOB_Engine/incomingOrders.txt");
	if(!f.is_open()){
		std::cout << "error opening file";
	return {} ;}
	std::string str;
	std::vector<InboundJob> vec;
	// ec added; when so, this function doesn't throw, it stores error into the ec.
	size_t sizeBytes = std::filesystem::file_size(filePath,ec); 
	if(!ec && sizeBytes > 0){
		size_t size = sizeBytes/15;
		vec.reserve(size);
	} 
	
	constexpr char delimiter = ',';
	while(std::getline(f,str)){
		InboundJob obj;
		std::string_view s(str);
		if(!isOrderCorrect(s)){continue;}
 
			
		size_t startPos = 0;
		size_t endPos = s.find(delimiter, startPos);
		std::string_view ID = s.substr(startPos,endPos-startPos);	
		auto res = std::from_chars(ID.data(),ID.data() + ID.size(),obj.orderId);
		if(res.ec != std::errc{}){continue;}
		
		startPos = endPos +1;
		endPos = s.find(delimiter,startPos);
		std::string_view side = s.substr(startPos,endPos-startPos);
		if(side == "B" )obj.side = Side::Buy ;
		else if(side == "S")obj.side = Side::Sell;
		else{continue;}
		
		startPos = endPos +1;
		endPos = s.find(delimiter,startPos);
		std::string_view Q = s.substr(startPos,endPos-startPos);
		auto res2 = std::from_chars(Q.data(),Q.data() + Q.size(), obj.quantity);
		if(res2.ec != std::errc{}){continue;}
		startPos = endPos +1;	
		std::string_view P = s.substr(startPos);
		auto res3 = std::from_chars(P.data(),P.data() + P.size(), obj.price);
		if(res3.ec != std::errc{}){continue;}
		if(obj.price < 1 || obj.price >= 300 || obj.quantity < 1 || obj.quantity > 9999 
				 || obj.orderId >= 20000) continue;
		addedorders++;
		vec.push_back(obj);
		
	};
	f.close();	
	return vec;
	
}


