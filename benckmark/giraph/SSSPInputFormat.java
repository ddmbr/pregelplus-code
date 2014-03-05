package org.apache.giraph.examples;

import java.io.IOException;
import java.util.List;
import java.util.regex.Pattern;

import org.apache.giraph.edge.Edge;
import org.apache.giraph.edge.EdgeFactory;
import org.apache.giraph.io.formats.TextVertexInputFormat;

import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.InputSplit;
import org.apache.hadoop.mapreduce.TaskAttemptContext;

import com.google.common.collect.Lists;

public class SSSPInputFormat extends
	TextVertexInputFormat<IntWritable, DoubleWritable, DoubleWritable> {
    /** Separator of the vertex and neighbors */
    private static final Pattern SEPARATOR = Pattern.compile("[\t ]");

    @Override
    public TextVertexReader createVertexReader(InputSplit split,
	    TaskAttemptContext context) throws IOException {
	return new IntIntNullVertexReader();
    }

    /**
     * Vertex reader associated with {@link IntIntNullTextInputFormat}.
     */
    public class IntIntNullVertexReader extends
	    TextVertexReaderFromEachLineProcessed<String[]> {

	private IntWritable id;

	@Override
	protected String[] preprocessLine(Text line) throws IOException {
	    String[] tokens = SEPARATOR.split(line.toString());
	    id = new IntWritable(Integer.parseInt(tokens[0]));
	    return tokens;
	}

	@Override
	protected IntWritable getId(String[] tokens) throws IOException {
	    return id;
	}

	@Override
	protected DoubleWritable getValue(String[] tokens) throws IOException {
	    return new DoubleWritable(1.0 * id.get());
	}

	@Override
	protected Iterable<Edge<IntWritable, DoubleWritable>> getEdges(
		String[] tokens) throws IOException {
	    List<Edge<IntWritable, DoubleWritable>> edges = Lists
		    .newArrayListWithCapacity((tokens.length - 2) / 2);
	    for (int n = 2; n < tokens.length; n += 2) {
		edges.add(EdgeFactory.create(
			new IntWritable(Integer.parseInt(tokens[n])),
			new DoubleWritable(Double.parseDouble(tokens[n + 1]))));
	    }
	    return edges;
	}
    }
}