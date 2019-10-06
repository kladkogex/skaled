/*
    Copyright (C) 2018-2019 SKALE Labs

    This file is part of skaled.

    skaled is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    skaled is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with skaled.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Eth.cpp
 * @authors:
 *   Sergiy Lavrynenko <sergiy@skalelabs.com>
 * @date 2019
 */

#include "SkaleStats.h"
#include "Eth.h"
#include <jsonrpccpp/common/exception.h>
#include <libweb3jsonrpc/JsonHelper.h>

#include <csignal>
#include <exception>

#include <skutils/console_colors.h>
#include <skutils/eth_utils.h>

namespace dev {
namespace rpc {

SkaleStats::SkaleStats( const nlohmann::json& joConfig, eth::Interface& _eth )
    : joConfig_( joConfig ), m_eth( _eth ) {
    nThisNodeIndex_ = findThisNodeIndex();
}

int SkaleStats::findThisNodeIndex() {
    try {
        if ( joConfig_.count( "skaleConfig" ) == 0 )
            throw std::runtime_error( "bad skaled config.json, cannot find \"skaleConfig\"" );
        const nlohmann::json& joSkaleConfig = joConfig_["skaleConfig"];
        //
        if ( joSkaleConfig.count( "nodeInfo" ) == 0 )
            throw std::runtime_error(
                "bad skaled config.json, cannot find \"skaleConfig\"/\"nodeInfo\"" );
        const nlohmann::json& joSkaleConfig_nodeInfo = joSkaleConfig["nodeInfo"];
        //
        if ( joSkaleConfig.count( "sChain" ) == 0 )
            throw std::runtime_error(
                "bad skaled config.json, cannot find \"skaleConfig\"/\"sChain\"" );
        const nlohmann::json& joSkaleConfig_sChain = joSkaleConfig["sChain"];
        //
        if ( joSkaleConfig_sChain.count( "nodes" ) == 0 )
            throw std::runtime_error(
                "bad skaled config.json, cannot find \"skaleConfig\"/\"sChain\"/\"nodes\"" );
        const nlohmann::json& joSkaleConfig_sChain_nodes = joSkaleConfig_sChain["nodes"];
        //
        int nID = joSkaleConfig_nodeInfo["nodeID"].get< int >();
        const nlohmann::json& jarrNodes = joSkaleConfig_sChain_nodes;
        size_t i, cnt = jarrNodes.size();
        for ( i = 0; i < cnt; ++i ) {
            const nlohmann::json& joNC = jarrNodes[i];
            try {
                int nWalkID = joNC["nodeID"].get< int >();
                if ( nID == nWalkID )
                    return joNC["schainIndex"].get< int >();
            } catch ( ... ) {
                continue;
            }
        }
    } catch ( ... ) {
    }
    return -1;
}

Json::Value SkaleStats::skale_stats() {
    try {
        nlohmann::json joStats = consumeSkaleStats();
        std::string strStatsJson = joStats.dump();
        Json::Value ret;
        Json::Reader().parse( strStatsJson, ret );
        return ret;
    } catch ( Exception const& ) {
        throw jsonrpc::JsonRpcException( exceptionToErrorMessage() );
    } catch ( const std::exception& ex ) {
        throw jsonrpc::JsonRpcException( ex.what() );
    }
}

Json::Value SkaleStats::skale_nodesRpcInfo() {
    try {
        if ( joConfig_.count( "skaleConfig" ) == 0 )
            throw std::runtime_error( "bad skaled config.json, cannot find \"skaleConfig\"" );
        const nlohmann::json& joSkaleConfig = joConfig_["skaleConfig"];
        //
        if ( joSkaleConfig.count( "nodeInfo" ) == 0 )
            throw std::runtime_error(
                "bad skaled config.json, cannot find \"skaleConfig\"/\"nodeInfo\"" );
        const nlohmann::json& joSkaleConfig_nodeInfo = joSkaleConfig["nodeInfo"];
        //
        if ( joSkaleConfig.count( "sChain" ) == 0 )
            throw std::runtime_error(
                "bad skaled config.json, cannot find \"skaleConfig\"/\"sChain\"" );
        const nlohmann::json& joSkaleConfig_sChain = joSkaleConfig["sChain"];
        //
        if ( joSkaleConfig_sChain.count( "nodes" ) == 0 )
            throw std::runtime_error(
                "bad skaled config.json, cannot find \"skaleConfig\"/\"sChain\"/\"nodes\"" );
        const nlohmann::json& joSkaleConfig_sChain_nodes = joSkaleConfig_sChain["nodes"];
        //
        nlohmann::json jo = nlohmann::json::object();
        //
        nlohmann::json joThisNode = nlohmann::json::object();
        joThisNode["thisNodeIndex"] = nThisNodeIndex_;  // 1-based "schainIndex"
        try {
            joThisNode["nodeID"] = joSkaleConfig_nodeInfo["nodeID"].get< int >();
        } catch ( ... ) {
            joThisNode["nodeID"] = -1;
        }
        try {
            joThisNode["bindIP"] = joSkaleConfig_nodeInfo["bindIP"].get< std::string >();
        } catch ( ... ) {
            joThisNode["bindIP"] = "";
        }
        try {
            joThisNode["bindIP6"] = joSkaleConfig_nodeInfo["bindIP6"].get< std::string >();
        } catch ( ... ) {
            joThisNode["bindIP6"] = "";
        }
        try {
            joThisNode["httpRpcPort"] = joSkaleConfig_nodeInfo["httpRpcPort"].get< int >();
        } catch ( ... ) {
            joThisNode["httpRpcPort"] = 0;
        }
        try {
            joThisNode["httpsRpcPort"] = joSkaleConfig_nodeInfo["httpsRpcPort"].get< int >();
        } catch ( ... ) {
            joThisNode["httpsRpcPort"] = 0;
        }
        try {
            joThisNode["wsRpcPort"] = joSkaleConfig_nodeInfo["wsRpcPort"].get< int >();
        } catch ( ... ) {
            joThisNode["wsRpcPort"] = 0;
        }
        try {
            joThisNode["wssRpcPort"] = joSkaleConfig_nodeInfo["wssRpcPort"].get< int >();
        } catch ( ... ) {
            joThisNode["wssRpcPort"] = 0;
        }
        try {
            joThisNode["httpRpcPort6"] = joSkaleConfig_nodeInfo["httpRpcPort6"].get< int >();
        } catch ( ... ) {
            joThisNode["httpRpcPort6"] = 0;
        }
        try {
            joThisNode["httpsRpcPort6"] = joSkaleConfig_nodeInfo["httpsRpcPort6"].get< int >();
        } catch ( ... ) {
            joThisNode["httpsRpcPort6"] = 0;
        }
        try {
            joThisNode["wsRpcPort6"] = joSkaleConfig_nodeInfo["wsRpcPort6"].get< int >();
        } catch ( ... ) {
            joThisNode["wsRpcPort6"] = 0;
        }
        try {
            joThisNode["wssRpcPort6"] = joSkaleConfig_nodeInfo["wssRpcPort6"].get< int >();
        } catch ( ... ) {
            joThisNode["wssRpcPort6"] = 0;
        }
        try {
            joThisNode["acceptors"] = joSkaleConfig_nodeInfo["acceptors"].get< int >();
        } catch ( ... ) {
            joThisNode["acceptors"] = 0;
        }
        try {
            joThisNode["enable-debug-behavior-apis"] =
                joSkaleConfig_nodeInfo["enable-debug-behavior-apis"].get< bool >();
        } catch ( ... ) {
            joThisNode["enable-debug-behavior-apis"] = false;
        }
        try {
            joThisNode["unsafe-transactions"] =
                joSkaleConfig_nodeInfo["unsafe-transactions"].get< bool >();
        } catch ( ... ) {
            joThisNode["unsafe-transactions"] = false;
        }
        //
        try {
            jo["schainID"] = joSkaleConfig_sChain["schainID"].get< int >();
        } catch ( ... ) {
            joThisNode["schainID"] = -1;
        }
        try {
            jo["schainName"] = joSkaleConfig_sChain["schainName"].get< std::string >();
        } catch ( ... ) {
            joThisNode["schainName"] = "";
        }
        nlohmann::json jarrNetwork = nlohmann::json::array();
        size_t i, cnt = joSkaleConfig_sChain_nodes.size();
        for ( i = 0; i < cnt; ++i ) {
            const nlohmann::json& joNC = joSkaleConfig_sChain_nodes[i];
            nlohmann::json joNode = nlohmann::json::object();
            try {
                joNode["nodeID"] = joNC["nodeID"].get< int >();
            } catch ( ... ) {
                joNode["nodeID"] = 1;
            }
            try {
                joNode["ip"] = joNC["ip"].get< std::string >();
            } catch ( ... ) {
                joNode["ip"] = "";
            }
            try {
                joNode["ip6"] = joNC["ip6"].get< std::string >();
            } catch ( ... ) {
                joNode["ip6"] = "";
            }
            try {
                joNode["schainIndex"] = joNC["schainIndex"].get< int >();
            } catch ( ... ) {
                joNode["schainIndex"] = -1;
            }
            try {
                joNode["httpRpcPort"] = joNC["httpRpcPort"].get< int >();
            } catch ( ... ) {
                joNode["httpRpcPort"] = 0;
            }
            try {
                joNode["httpsRpcPort"] = joNC["httpsRpcPort"].get< int >();
            } catch ( ... ) {
                joNode["httpsRpcPort"] = 0;
            }
            try {
                joNode["wsRpcPort"] = joNC["wsRpcPort"].get< int >();
            } catch ( ... ) {
                joNode["wsRpcPort"] = 0;
            }
            try {
                joNode["wssRpcPort"] = joNC["wssRpcPort"].get< int >();
            } catch ( ... ) {
                joNode["wssRpcPort"] = 0;
            }
            try {
                joNode["httpRpcPort6"] = joNC["httpRpcPort6"].get< int >();
            } catch ( ... ) {
                joNode["httpRpcPort6"] = 0;
            }
            try {
                joNode["httpsRpcPort6"] = joNC["httpsRpcPort6"].get< int >();
            } catch ( ... ) {
                joNode["httpsRpcPort6"] = 0;
            }
            try {
                joNode["wsRpcPort6"] = joNC["wsRpcPort6"].get< int >();
            } catch ( ... ) {
                joNode["wsRpcPort6"] = 0;
            }
            try {
                joNode["wssRpcPort6"] = joNC["wssRpcPort6"].get< int >();
            } catch ( ... ) {
                joNode["wssRpcPort6"] = 0;
            }
            //
            jarrNetwork.push_back( joNode );
        }
        //
        jo["node"] = joThisNode;
        jo["network"] = jarrNetwork;
        jo["node"] = joThisNode;
        std::string s = jo.dump();
        Json::Value ret;
        Json::Reader().parse( s, ret );
        return ret;
    } catch ( Exception const& ) {
        throw jsonrpc::JsonRpcException( exceptionToErrorMessage() );
    } catch ( const std::exception& ex ) {
        throw jsonrpc::JsonRpcException( ex.what() );
    }
}

Json::Value SkaleStats::skale_imaInfo() {
    try {
        if ( joConfig_.count( "skaleConfig" ) == 0 )
            throw std::runtime_error( "bad skaled config.json, cannot find \"skaleConfig\"" );
        const nlohmann::json& joSkaleConfig = joConfig_["skaleConfig"];
        //
        if ( joSkaleConfig.count( "nodeInfo" ) == 0 )
            throw std::runtime_error(
                "bad skaled config.json, cannot find \"skaleConfig\"/\"nodeInfo\"" );
        const nlohmann::json& joSkaleConfig_nodeInfo = joSkaleConfig["nodeInfo"];
        //
        if ( joSkaleConfig_nodeInfo.count( "wallets" ) == 0 )
            throw std::runtime_error(
                "bad skaled config.json, cannot find \"skaleConfig\"/\"nodeInfo\"/\"wallets\"" );
        const nlohmann::json& joSkaleConfig_nodeInfo_wallets = joSkaleConfig_nodeInfo["wallets"];
        //
        if ( joSkaleConfig_nodeInfo_wallets.count( "ima" ) == 0 )
            throw std::runtime_error(
                "bad skaled config.json, cannot find "
                "\"skaleConfig\"/\"nodeInfo\"/\"wallets\"/\"ima\"" );
        const nlohmann::json& joSkaleConfig_nodeInfo_wallets_ima =
            joSkaleConfig_nodeInfo_wallets["ima"];
        //
        nlohmann::json jo = nlohmann::json::object();
        //
        jo["thisNodeIndex"] = nThisNodeIndex_;  // 1-based "schainIndex"
        jo["insecureCommonBLSPublicKey0"] =
            joSkaleConfig_nodeInfo_wallets_ima["insecureCommonBLSPublicKey0"];
        jo["insecureCommonBLSPublicKey1"] =
            joSkaleConfig_nodeInfo_wallets_ima["insecureCommonBLSPublicKey1"];
        jo["insecureCommonBLSPublicKey2"] =
            joSkaleConfig_nodeInfo_wallets_ima["insecureCommonBLSPublicKey2"];
        jo["insecureCommonBLSPublicKey3"] =
            joSkaleConfig_nodeInfo_wallets_ima["insecureCommonBLSPublicKey3"];
        //
        std::string s = jo.dump();
        Json::Value ret;
        Json::Reader().parse( s, ret );
        return ret;
    } catch ( Exception const& ) {
        throw jsonrpc::JsonRpcException( exceptionToErrorMessage() );
    } catch ( const std::exception& ex ) {
        throw jsonrpc::JsonRpcException( ex.what() );
    }
}

Json::Value SkaleStats::skale_imaVerifyAndSign( const Json::Value& request ) {
    try {
    } catch ( Exception const& ) {
        throw jsonrpc::JsonRpcException( exceptionToErrorMessage() );
    } catch ( const std::exception& ex ) {
        throw jsonrpc::JsonRpcException( ex.what() );
    }
}

};  // namespace rpc
};  // namespace dev
