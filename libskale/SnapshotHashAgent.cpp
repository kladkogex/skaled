/*
    Copyright (C) 2019-present, SKALE Labs

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
/**
 * @file SnapshotHashAgent.cpp
 * @author oleh Nikolaiev
 * @date 2019
 */

#include "SnapshotHashAgent.h"

#include <libethcore/CommonJS.h>
#include <libweb3jsonrpc/Skale.h>
#include <skutils/rest_call.h>

unsigned SnapshotHashAgent::getBlockNumber( const std::string& strURLWeb3 ) {
    skutils::rest::client cli;
    if ( !cli.open( strURLWeb3 ) ) {
        throw std::runtime_error( "REST failed to connect to server" );
    }

    nlohmann::json joIn = nlohmann::json::object();
    joIn["jsonrpc"] = "2.0";
    joIn["method"] = "eth_blockNumber";
    joIn["params"] = nlohmann::json::object();
    skutils::rest::data_t d = cli.call( joIn );
    if ( d.empty() ) {
        throw std::runtime_error( "cannot get blockNumber to download snapshot" );
    }
    nlohmann::json joAnswer = nlohmann::json::parse( d.s_ );
    this->block_number_ = dev::eth::jsToBlockNumber( joAnswer["result"].get< std::string >() );
    this->block_number_ -= this->block_number_ % this->chain_params_.nodeInfo.snapshotInterval;

    return this->block_number_;
}

dev::h256 SnapshotHashAgent::voteForHash() {
    std::map< dev::h256, size_t > map_hash;
    for ( size_t i = 0; i < this->n_; ++i ) {
        if ( this->chain_params_.nodeInfo.ip == this->chain_params_.sChain.nodes[i].ip ) {
            continue;
        }

        map_hash[this->hashes_[i]] += 1;
    }

    for ( const auto& hash : map_hash ) {
        if ( 3 * hash.second > 2 * ( this->n_ + 2 ) ) {
            for ( size_t i = 0; i < this->n_; ++i ) {
                if ( this->chain_params_.nodeInfo.ip == this->chain_params_.sChain.nodes[i].ip ) {
                    continue;
                }

                if ( this->hashes_[i] == hash.first ) {
                    this->nodes_to_download_snapshot_from_.push_back( i );
                }
            }
            return hash.first;
        }
    }

    throw std::logic_error( "note enough votes to choose hash" );
}

void SnapshotHashAgent::getHashFromOthers() {
    for ( size_t i = 0; i < this->n_; ++i ) {
        if ( this->chain_params_.nodeInfo.ip == this->chain_params_.sChain.nodes[i].ip ) {
            continue;
        }

        std::thread( [this, i]() {
            nlohmann::json joCall = nlohmann::json::object();
            joCall["jsonrpc"] = "2.0";
            joCall["method"] = "skale_getSnapshotHash";
            nlohmann::json obj = {this->block_number_};
            joCall["params"] = obj;
            skutils::rest::client cli( this->chain_params_.sChain.nodes[i].ip );
            skutils::rest::data_t d = cli.call( joCall );
            if ( d.empty() ) {
                throw std::runtime_error( "Main Net call to skale_getSnapshotHash failed" );
            }
            std::string str_hash = nlohmann::json::parse( d.s_ )["result"];
            this->hashes_[i] = dev::h256( str_hash );
        } )
            .detach();
    }
}

std::string SnapshotHashAgent::getNodeToDownloadSnapshotFrom() {
    if ( this->idx_ == this->n_ ) {
        throw std::logic_error(
            "allready tried to download snapshot from all nodes with voted hash" );
    }

    return this->chain_params_.sChain.nodes[this->nodes_to_download_snapshot_from_[this->idx_++]]
        .ip;
}