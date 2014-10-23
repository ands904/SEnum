object Form1: TForm1
  Left = 101
  Top = 44
  Width = 433
  Height = 227
  Caption = #1055#1077#1095#1072#1090#1100' '#1092#1072#1081#1083#1072' PDF '#1074' '#1076#1074#1072' '#1087#1086#1090#1086#1082#1072' - '#1040'3 '#1080' '#1040'0'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnDestroy = FormDestroy
  OnPaint = FormPaint
  PixelsPerInch = 96
  TextHeight = 13
  object Notebook1: TNotebook
    Left = 0
    Top = 0
    Width = 425
    Height = 200
    Align = alClient
    PageIndex = 1
    TabOrder = 0
    object TPage
      Left = 0
      Top = 0
      Caption = 'Debug'
      DesignSize = (
        425
        200)
      object Button1: TButton
        Left = 32
        Top = 65
        Width = 75
        Height = 25
        Caption = 'Start'
        TabOrder = 0
        OnClick = Button1Click
      end
      object Button2: TButton
        Left = 336
        Top = 64
        Width = 75
        Height = 25
        Caption = 'Stop'
        TabOrder = 1
        OnClick = Button2Click
      end
      object Edit1: TEdit
        Left = 16
        Top = 8
        Width = 385
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 2
        Text = 'GetClassName'
        OnKeyDown = Edit1KeyDown
      end
      object Edit2: TEdit
        Left = 16
        Top = 34
        Width = 385
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 3
        Text = 'GetClassName'
        OnKeyDown = Edit1KeyDown
      end
      object Button3: TButton
        Left = 120
        Top = 65
        Width = 75
        Height = 25
        Caption = 'SaveTiff'
        TabOrder = 4
        OnClick = Button3Click
      end
      object Button4: TButton
        Left = 121
        Top = 130
        Width = 75
        Height = 25
        Caption = 'CreatePDF'
        TabOrder = 5
        OnClick = Button4Click
      end
      object Button5: TButton
        Left = 32
        Top = 96
        Width = 75
        Height = 25
        Caption = 'ExpandChild'
        TabOrder = 6
        OnClick = Button5Click
      end
      object Button6: TButton
        Left = 32
        Top = 128
        Width = 75
        Height = 25
        Caption = 'TempDir'
        TabOrder = 7
        OnClick = Button6Click
      end
      object Button7: TButton
        Left = 120
        Top = 96
        Width = 75
        Height = 25
        Caption = 'ClassifyTiff'
        TabOrder = 8
        OnClick = Button7Click
      end
      object Button8: TButton
        Left = 208
        Top = 64
        Width = 75
        Height = 25
        Caption = 'Print'
        TabOrder = 9
        OnClick = Button8Click
      end
      object Button9: TButton
        Left = 208
        Top = 96
        Width = 75
        Height = 25
        Caption = #1055#1088#1086#1090#1086#1082#1086#1083
        TabOrder = 10
        OnClick = Button9Click
      end
      object TestOLE: TButton
        Left = 96
        Top = 160
        Width = 75
        Height = 25
        Caption = 'TestOLE'
        TabOrder = 11
        OnClick = TestOLEClick
      end
      object Button10: TButton
        Left = 184
        Top = 160
        Width = 75
        Height = 25
        Caption = 'TestPrinter'
        TabOrder = 12
        OnClick = Button10Click
      end
    end
    object TPage
      Left = 0
      Top = 0
      Caption = 'Work'
      DesignSize = (
        425
        200)
      object Label1: TLabel
        Left = 8
        Top = 8
        Width = 84
        Height = 13
        Caption = #1048#1084#1103' '#1092#1072#1081#1083#1072' PDF:'
      end
      object SelectFileSpeedButton: TSpeedButton
        Left = 398
        Top = 4
        Width = 23
        Height = 22
        Anchors = [akTop, akRight]
        Caption = '...'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -16
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
        OnClick = SelectFileSpeedButtonClick
      end
      object Label2: TLabel
        Left = 357
        Top = 93
        Width = 34
        Height = 13
        Caption = #1050#1086#1087#1080#1081':'
      end
      object Label3: TLabel
        Left = 357
        Top = 139
        Width = 34
        Height = 13
        Caption = #1050#1086#1087#1080#1081':'
      end
      object Label4: TLabel
        Left = 357
        Top = 49
        Width = 34
        Height = 13
        Caption = #1050#1086#1087#1080#1081':'
      end
      object P4SpeedButton: TSpeedButton
        Left = 324
        Top = 43
        Width = 23
        Height = 22
        Caption = '...'
        OnClick = P4SpeedButtonClick
      end
      object P3SpeedButton: TSpeedButton
        Left = 324
        Top = 87
        Width = 23
        Height = 22
        Caption = '...'
        OnClick = P3SpeedButtonClick
      end
      object P0SpeedButton: TSpeedButton
        Left = 324
        Top = 131
        Width = 23
        Height = 22
        Caption = '...'
        OnClick = P0SpeedButtonClick
      end
      object Label5: TLabel
        Left = 11
        Top = 72
        Width = 16
        Height = 13
        Caption = 'A3:'
      end
      object CreateA4CheckBox: TCheckBox
        Left = 11
        Top = 27
        Width = 137
        Height = 17
        Caption = #1057#1086#1079#1076#1072#1074#1072#1090#1100' '#1092#1072#1081#1083' A4'
        Checked = True
        State = cbChecked
        TabOrder = 10
        OnClick = CreateA4CheckBoxClick
      end
      object CreateA3CheckBox: TCheckBox
        Left = 16
        Top = 167
        Width = 89
        Height = 17
        Caption = #1057#1086#1079#1076#1072#1074#1072#1090#1100' '#1092#1072#1081#1083' A3'
        Checked = True
        State = cbChecked
        TabOrder = 0
        Visible = False
        OnClick = CreateA3CheckBoxClick
      end
      object FilnEdit: TEdit
        Left = 96
        Top = 4
        Width = 298
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 1
      end
      object PrintA4CheckBox: TCheckBox
        Left = 44
        Top = 49
        Width = 121
        Height = 17
        Caption = #1055#1077#1095#1072#1090#1072#1090#1100' '#1092#1072#1081#1083' A4'
        TabOrder = 12
      end
      object PrintA3CheckBox: TCheckBox
        Left = 44
        Top = 93
        Width = 121
        Height = 17
        Caption = #1055#1077#1095#1072#1090#1072#1090#1100' '#1092#1072#1081#1083' A3'
        TabOrder = 2
      end
      object CreateA0CheckBox: TCheckBox
        Left = 11
        Top = 116
        Width = 129
        Height = 17
        Caption = #1057#1086#1079#1076#1072#1074#1072#1090#1100' '#1092#1072#1081#1083' '#1040'0'
        Checked = True
        State = cbChecked
        TabOrder = 3
        OnClick = CreateA0CheckBoxClick
      end
      object PrintA0CheckBox: TCheckBox
        Left = 44
        Top = 139
        Width = 97
        Height = 17
        Caption = #1055#1077#1095#1072#1090#1072#1090#1100' '#1092#1072#1081#1083' '#1040'0'
        TabOrder = 4
      end
      object OkButton: TButton
        Left = 127
        Top = 171
        Width = 75
        Height = 25
        Anchors = [akBottom]
        Caption = 'Ok'
        TabOrder = 5
        OnClick = OkButtonClick
      end
      object CancelButton: TButton
        Left = 215
        Top = 171
        Width = 75
        Height = 25
        Anchors = [akBottom]
        Caption = #1054#1090#1084#1077#1085#1072
        TabOrder = 6
        OnClick = CancelButtonClick
      end
      object A4CopiesEdit: TEdit
        Left = 394
        Top = 46
        Width = 25
        Height = 21
        TabOrder = 7
        Text = '1'
      end
      object A3CopiesEdit: TEdit
        Left = 394
        Top = 90
        Width = 25
        Height = 21
        TabOrder = 11
        Text = '1'
      end
      object A0CopiesEdit: TEdit
        Left = 394
        Top = 135
        Width = 25
        Height = 21
        TabOrder = 8
        Text = '1'
      end
      object RepeatEdit: TEdit
        Left = 336
        Top = 168
        Width = 57
        Height = 21
        TabOrder = 9
        Text = '1'
        Visible = False
      end
      object P4Edit: TEdit
        Left = 162
        Top = 44
        Width = 159
        Height = 21
        ReadOnly = True
        TabOrder = 13
      end
      object P3Edit: TEdit
        Left = 162
        Top = 88
        Width = 159
        Height = 21
        ReadOnly = True
        TabOrder = 14
      end
      object P0Edit: TEdit
        Left = 162
        Top = 132
        Width = 159
        Height = 21
        ReadOnly = True
        TabOrder = 15
      end
      object A3ComboBox: TComboBox
        Left = 30
        Top = 68
        Width = 132
        Height = 21
        Style = csDropDownList
        ItemHeight = 13
        ItemIndex = 1
        TabOrder = 16
        Text = #1057#1086#1079#1076#1072#1074#1072#1090#1100' '#1086#1090#1076#1077#1083#1100#1085#1086
        Items.Strings = (
          #1053#1077' '#1089#1086#1079#1076#1072#1074#1072#1090#1100
          #1057#1086#1079#1076#1072#1074#1072#1090#1100' '#1086#1090#1076#1077#1083#1100#1085#1086
          #1055#1088#1080#1089#1086#1077#1076#1080#1085#1080#1090#1100' '#1082' A4'
          #1055#1088#1080#1089#1086#1077#1076#1080#1085#1080#1090#1100' '#1082' '#1040'0')
      end
    end
  end
  object OpenDialog1: TOpenDialog
    DefaultExt = 'pdf'
    Filter = #1060#1072#1081#1083#1099' *.pdf|*.pdf'
    Options = [ofHideReadOnly, ofPathMustExist, ofFileMustExist, ofEnableSizing]
    Left = 304
    Top = 160
  end
  object PrintDialog1: TPrintDialog
    Left = 384
    Top = 160
  end
  object PrinterSetupDialog1: TPrinterSetupDialog
    Left = 344
    Top = 160
  end
end
