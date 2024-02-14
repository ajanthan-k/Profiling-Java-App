/*
 * Copyright (c) 2014, Oracle America, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *  * Neither the name of Oracle nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

package uk.ac.ic.doc.spe;

import org.openjdk.jmh.annotations.Benchmark;
import org.openjdk.jmh.annotations.Param;
import org.openjdk.jmh.annotations.Setup;
import org.openjdk.jmh.annotations.State;
import org.openjdk.jmh.annotations.Scope;
import org.openjdk.jmh.runner.Runner;
import org.openjdk.jmh.runner.options.Options;
import org.openjdk.jmh.runner.options.OptionsBuilder;
import tech.tablesaw.api.StringColumn;
import tech.tablesaw.api.DoubleColumn;
import tech.tablesaw.api.Table;
import static tech.tablesaw.aggregate.AggregateFunctions.mean;

import java.util.Random;

@State(Scope.Benchmark)
public class MyBenchmark {

    private Table syntheticData;

    @Param({"0.1", "0.3", "0.5", "0.7", "0.9"})
    private double skewness;

    @Setup
    public void setup() {
        syntheticData = generateTable(100000, skewness);
    }

    @Benchmark
    public Table benchmarkSummarizeBy() {
        return syntheticData.summarize("Value", mean).by("Category");
    }

    private static Table generateTable(int rowCount, double skewness) {
        Random random = new Random(19);
        String[] categories = new String[rowCount];
        double[] values = new double[rowCount];

        for (int i = 0; i < rowCount; i++) {
            double bias = Math.pow(1 - skewness, 1); // increase for more pronounved skewness
            int categoryIndex = (int) Math.floor(CATEGORIES.length * Math.pow(random.nextDouble(), bias));
            
            categories[i] = CATEGORIES[Math.min(categoryIndex, CATEGORIES.length - 1)];
            values[i] = random.nextDouble() * 100;
        }

        return Table.create("Synthetic Data")
                    .addColumns(StringColumn.create("Category", categories),
                                DoubleColumn.create("Value", values));
    }

    private static final String[] CATEGORIES = generateCategories(1000);

    private static String[] generateCategories(int count) {
        String[] categories = new String[count];
        for (int i = 0; i < count; i++) {
            categories[i] = "Cat_" + i;
        }
        return categories;
    }

    public static void main(String[] args) throws Exception {
        Options opt = new OptionsBuilder()
                .include(MyBenchmark.class.getSimpleName())
                .forks(1)
                .build();

        new Runner(opt).run();
    }
}

