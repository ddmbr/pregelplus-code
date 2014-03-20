package gps.examples.benchmark;
import org.apache.commons.cli.CommandLine;

import gps.globalobjects.BooleanANDGlobalObject;
import gps.graph.Master;


/**
 * Master class for the extended sssp algorithm. It coordinates the flow of the
 * two stages of the algorithm as well as computing the average distance at the
 * very end of the computation.
 * 
 * @author semihsalihoglu
 */
public class SVMaster extends Master {
    public SVMaster(CommandLine line) {
	java.util.HashMap<String, String> arg_map = new java.util.HashMap<String, String>();
	gps.node.Utils.parseOtherOptions(line, arg_map);
    }
    @Override
    public void compute(int superstepNo) {	
	getGlobalObjectsMap().clearNonDefaultObjects();
	getGlobalObjectsMap().putGlobalObject("Star", new BooleanANDGlobalObject(false));
	
    }

}