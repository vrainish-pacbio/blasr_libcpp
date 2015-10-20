// Copyright (c) 2014-2015, Pacific Biosciences of California, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted (subject to the limitations in the
// disclaimer below) provided that the following conditions are met:
//
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//  * Neither the name of Pacific Biosciences nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY PACIFIC
// BIOSCIENCES AND ITS CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL PACIFIC BIOSCIENCES OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

// Author: Yuan Li

#include "RegionTypeMap.hpp"

std::string RegionTypeMap::ToString(RegionType rt) {
    assert(RegionTypeToString.find(rt) != RegionTypeToString.end());
    return RegionTypeToString.find(rt)->second;
}

RegionType RegionTypeMap::ToRegionType(const std::string & str) {
    if (StringToRegionType.find(str) == StringToRegionType.end()) {
        std::cout << "Unsupported RegionType " << str << std::endl;
        assert(false);
    }
    return StringToRegionType.find(str)->second;
}

int RegionTypeMap::ToIndex(const std::string & typeStr, const std::vector<std::string> & typeStrs) {
    auto it = std::find(typeStrs.begin(), typeStrs.end(), typeStr);
    if (it == typeStrs.end()) {
        std::cout << "Could not find RegionType " << typeStr << std::endl;
        assert(false);
    } else {
        return std::distance(typeStrs.begin(), it);
    }
}

int RegionTypeMap::ToIndex(RegionType rt, const std::vector<std::string> & typeStrs) {
    return RegionTypeMap::ToIndex(RegionTypeMap::ToString(rt), typeStrs);
}

int RegionTypeMap::ToIndex(RegionType rt, const std::vector<RegionType> & regionTypes) {
    auto it = std::find(regionTypes.begin(), regionTypes.end(), rt);
    if (it == regionTypes.end()) {
        std::cout << "Could not find RegionType " << RegionTypeMap::ToString(rt) << std::endl;
        assert(false);
    } else {
        return std::distance(regionTypes.begin(), it);
    }
}

const std::map<RegionType, std::string> RegionTypeMap::RegionTypeToString = {
    {Adapter,  "Adapter"},
    {Insert,   "Insert"},
    {HQRegion, "HQRegion"},
    {BarCode,  "Barcode"}
};

const std::map<std::string, RegionType> RegionTypeMap::StringToRegionType = {
    {"Adapter",  Adapter},
    {"Insert",   Insert},
    {"HQRegion", HQRegion},
    {"Barcode",  BarCode},
};