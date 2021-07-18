'''
Created on Jul 8, 2021

@author: mballance
'''
import argparse
import tblink_rpc_core
import asyncio
from tblink_rpc_core.json.json_transport import JsonTransport
from tblink_rpc_core.endpoint import Endpoint
import sys
from tblink.impl.endpoint_rgy import EndpointRgy

def getparser():
    parser = argparse.ArgumentParser()
    
    parser.add_argument("port")
    parser.add_argument("--host", default="localhost")
    
    return parser

def main():
    parser = getparser()
    
    args = parser.parse_args()
        
    print("Connecting to %s:%d" % (args.host, int(args.port)))
    sys.stdout.flush()
    
    loop = asyncio.get_event_loop()
    reader, writer = loop.run_until_complete(
        asyncio.open_connection(args.host, int(args.port)))
    print("reader=" + str(reader))
    sys.stdout.flush()

    # TODO: endpoint needs to have services
    transport = JsonTransport(reader, writer)
    endpoint = Endpoint(transport)
    
    EndpointRgy.inst().add_endpoint(endpoint)

    # Start the run loop for the transport
    print("--> starting recv loop")
    sys.stdout.flush()
    asyncio.ensure_future(transport.run())
    print("<-- starting recv loop")
    sys.stdout.flush()
   
    # TODO: this needs to be managed by the
    # pytblink package so the appropriate
    # Python phases can be run
    print("--> Python::build_complete")
    sys.stdout.flush()
    loop.run_until_complete(endpoint.build_complete())
    print("<-- Python::build_complete")    
    sys.stdout.flush()
    
    ev = asyncio.Event()
    count = 0
    async def cb_f():
        nonlocal endpoint
        nonlocal count
        
        count += 1
        
        print("time_cb_f %d" % count)
        sys.stdout.flush()
        

        if count < 16:
            print("--> calling add_time_callback")        
            await endpoint.add_time_callback(10, cb_f)
            print("<-- calling add_time_callback")        
        else:
            await endpoint.shutdown()
            ev.set()
            
    id = loop.run_until_complete(endpoint.add_time_callback(10, cb_f))
    
    print("--> Python::connect_complete")
    sys.stdout.flush()
    loop.run_until_complete(endpoint.connect_complete())
    print("<-- Python::connect_complete")    
    sys.stdout.flush()

    print("--> wait_callback")
    sys.stdout.flush()    
    loop.run_until_complete(ev.wait())
    print("<-- wait_callback")
    sys.stdout.flush()    
    
    loop.run_until_complete(endpoint.shutdown())
    

if __name__ == "__main__":
    main()
    