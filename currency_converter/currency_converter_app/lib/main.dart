import 'package:currency_converter/currency_converter_cupertino_page.dart';
import 'package:currency_converter/currency_converter_material_page.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';

void main() {
  runApp(const MyApp());
}

// Types of Widgets
// 1. StatelessWidget
// 2. StatefulWidget

// State

// 1. Material Design
// 2. Cupertino Design

// This is the main application widget for Material Design
class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key); // Key is used to uniquely identify widgets

  @override
  Widget build(BuildContext context) {
    // MaterialApp is a widget that provides Material Design specific functionality
    return const MaterialApp(
      home: CurrencyConverterMaterialPage(), // Setting the home page to CurrencyConverterMaterialPage
    );
  }
}

// This is the main application widget for Cupertino Design
class MyCupertinoApp extends StatelessWidget {
  const MyCupertinoApp({Key? key}) : super(key: key); // Key is used to uniquely identify widgets

  @override
  Widget build(BuildContext context) {
    // CupertinoApp is a widget that provides Cupertino Design specific functionality
    return const CupertinoApp(
      home: CurrencyConverterCupertinoPage(), // Setting the home page to CurrencyConverterCupertinoPage
    );
  }
}
