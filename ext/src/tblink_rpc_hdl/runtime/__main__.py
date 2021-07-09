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

    transport = JsonTransport(reader, writer)
    endpoint = Endpoint(transport)

    # Start the run loop for the transport
    print("--> starting recv loop")
    sys.stdout.flush()
    asyncio.ensure_future(transport.run())
    print("<-- starting recv loop")
    sys.stdout.flush()
    
    print("--> Python::build_complete")
    sys.stdout.flush()
    loop.run_until_complete(endpoint.build_complete())
    print("<-- Python::build_complete")    
    sys.stdout.flush()
    print("--> Python::connect_complete")    
    sys.stdout.flush()
    loop.run_until_complete(endpoint.connect_complete())
    print("<-- Python::connect_complete")    
    sys.stdout.flush()

    ev = asyncio.Event()
    def cb_f():
        print("time_cb_f")
        sys.stdout.flush()
        ev.set()
            
    id = loop.run_until_complete(endpoint.add_time_callback(10, cb_f))

    print("--> wait_callback")
    sys.stdout.flush()    
    loop.run_until_complete(ev.wait())
    print("<-- wait_callback")
    sys.stdout.flush()    
    

if __name__ == "__main__":
    main()
    