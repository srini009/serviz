/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include "ams/NodeHandle.hpp"
#include "ams/RequestResult.hpp"
#include "ams/Exception.hpp"

#include "AsyncRequestImpl.hpp"
#include "ClientImpl.hpp"
#include "NodeHandleImpl.hpp"

#include <thallium/serialization/stl/string.hpp>
#include <thallium/serialization/stl/pair.hpp>
#include <conduit.hpp>

namespace ams {

NodeHandle::NodeHandle() = default;

NodeHandle::NodeHandle(const std::shared_ptr<NodeHandleImpl>& impl)
: self(impl) {}

NodeHandle::NodeHandle(const NodeHandle&) = default;

NodeHandle::NodeHandle(NodeHandle&&) = default;

NodeHandle& NodeHandle::operator=(const NodeHandle&) = default;

NodeHandle& NodeHandle::operator=(NodeHandle&&) = default;

NodeHandle::~NodeHandle() = default;

NodeHandle::operator bool() const {
    return static_cast<bool>(self);
}

Client NodeHandle::client() const {
    return Client(self->m_client);
}

void NodeHandle::sayHello() const {
    if(not self) throw Exception("Invalid ams::NodeHandle object");
    auto& rpc = self->m_client->m_say_hello;
    auto& ph  = self->m_ph;
    auto& node_id = self->m_node_id;
    rpc.on(ph)(node_id);
}

/* SR: Core Ascent APIs */
void NodeHandle::ams_open(conduit::Node opts) const {
    if(not self) throw Exception("Invalid ams::NodeHandle object");
    auto& rpc = self->m_client->m_ams_open;
    auto& ph  = self->m_ph;
    auto& node_id = self->m_node_id;
    RequestResult<bool> result = rpc.on(ph)(node_id, opts.to_yaml());
    if(not result.success()) {
        throw Exception(result.error());
    }
}

void NodeHandle::ams_publish(conduit::Node bp_mesh) const {
    if(not self) throw Exception("Invalid ams::NodeHandle object");
    auto& rpc = self->m_client->m_ams_publish;
    auto& ph  = self->m_ph;
    auto& node_id = self->m_node_id;
    RequestResult<bool> result = rpc.on(ph)(node_id, bp_mesh.to_yaml());
    if(not result.success()) {
        throw Exception(result.error());
    }
}

void NodeHandle::ams_execute(conduit::Node actions) const {
    if(not self) throw Exception("Invalid ams::NodeHandle object");
    auto& rpc = self->m_client->m_ams_execute;
    auto& ph  = self->m_ph;
    auto& node_id = self->m_node_id;
    RequestResult<bool> result = rpc.on(ph)(node_id, actions.to_yaml());
    if(not result.success()) {
        throw Exception(result.error());
    }
}

void NodeHandle::ams_close() const {
    if(not self) throw Exception("Invalid ams::NodeHandle object");
    auto& rpc = self->m_client->m_ams_close;
    auto& ph  = self->m_ph;
    auto& node_id = self->m_node_id;
    RequestResult<bool> result = rpc.on(ph)(node_id);
    if(not result.success()) {
        throw Exception(result.error());
    }
}
/* SR: Core Ascent APIs */

void NodeHandle::computeSum(
        int32_t x, int32_t y,
        int32_t* result,
        AsyncRequest* req) const
{
    if(not self) throw Exception("Invalid ams::NodeHandle object");
    auto& rpc = self->m_client->m_compute_sum;
    auto& ph  = self->m_ph;
    auto& node_id = self->m_node_id;
    if(req == nullptr) { // synchronous call
        RequestResult<int32_t> response = rpc.on(ph)(node_id, x, y);
        if(response.success()) {
            if(result) *result = response.value();
        } else {
            throw Exception(response.error());
        }
    } else { // asynchronous call
        auto async_response = rpc.on(ph).async(node_id, x, y);
        auto async_request_impl =
            std::make_shared<AsyncRequestImpl>(std::move(async_response));
        async_request_impl->m_wait_callback =
            [result](AsyncRequestImpl& async_request_impl) {
                RequestResult<int32_t> response =
                    async_request_impl.m_async_response.wait();
                    if(response.success()) {
                        if(result) *result = response.value();
                    } else {
                        throw Exception(response.error());
                    }
            };
        *req = AsyncRequest(std::move(async_request_impl));
    }
}

}