/*
 * -*- coding: utf-8 -*-
 *
 * jomiel-examples
 *
 * Copyright
 *  2019-2021 Toni Gündoğdu
 *
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package jomiel.examples;

import static java.lang.System.exit;

import picocli.CommandLine;

@SuppressWarnings("PMD.DoNotCallSystemExit")
public final class App {
  public static void main(final String[] args) {
    exit(new CommandLine(new Options()).execute(args));
  }
}
